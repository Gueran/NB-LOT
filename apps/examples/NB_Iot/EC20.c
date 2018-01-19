#include <nuttx/config.h>
#include <sys/ioctl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sched.h>
#include <errno.h>
#include <nuttx/fs/fs.h>
#include <nuttx/leds/userled.h>
#include <nuttx/board.h>
#include <sys/boardctl.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <termios.h>
#include <strings.h>
#include "datetype.h"
#include "GPIO.h"

#include "EC20.h"

static	int  		  fd_EC20_USART;
static	int  		  fd_EC20_USART_GPIO_INT;

static	int  		  fd_EC20_CPU_RST_NB;
static	int  		  fd_EC20_NB_PWR_EN;
static	int  		  fd_EC20_CPU_PWR_LTE;
static	int  		  fd_EC20_LTE_AP_READY;
static	int  		  fd_EC20_LTE_DTR;
static char   			EC20_rcvd_buf[128];
static  UInt8 check_ack_timeout = 10;

static void delay_ms(UInt16 ms)
{				  
	usleep(ms * 1000L);
} 

//检查返回的响应是否符合预期
//传入参数为预期返回的字符串
//返回0，为检测到预期值
//其他值，预期字符所在的位置
UInt8* EC20_check_ack(char *str)
{
	char *strx=0;
	
	read(fd_EC20_USART, EC20_rcvd_buf, sizeof(EC20_rcvd_buf));
	printf("recv:%s\n",EC20_rcvd_buf);
	strx=strstr((const char*)EC20_rcvd_buf,(const char*)str);
	return (UInt8*)strx;
}

//发生at指令函数
//cmd:at指令，ack：预期响应，waittime,超时时间
//返回0，发送成功
//返回1，发送超时
UInt8 EC20_send_cmd(char *cmd,char *ack,uint16_t waittime)
{
	UInt8 res=0; 
	fd_set 	rfds;
	struct timeval timeout;
	int  	iRet = 0;
	char  sendcmd[1024];
	
	sprintf(sendcmd,"%s\r\n",cmd);

	printf("|%s| \r\n",sendcmd);

	printf("send:%d\n",write(fd_EC20_USART,sendcmd,strlen(sendcmd)));

	delay_ms(100);
	if(ack&&waittime)
	{
			FD_ZERO(&rfds);											
			FD_SET(fd_EC20_USART, &rfds);									
			timeout.tv_sec = waittime;
			timeout.tv_usec = 0;
			iRet = select(fd_EC20_USART+1, &rfds, NULL, NULL, &timeout);  	//recv-timeout
			if (iRet < 0) 
			{
				printf("EC20_send_cmd select error!!!\n");
				return 1;
			}
			else if(iRet == 0)
			{
				printf("EC20_send_cmd rcv timeout!!!\n");
				return 1;	

			}
			else
			{
				if(FD_ISSET(fd_EC20_USART, &rfds)) 
				{
					delay_ms(100);
					if(EC20_check_ack(ack))return 0;
				}
			}
			
	}
	return 1;	
} 


void EC20_GPIO_Init()
{
	fd_EC20_CPU_RST_NB = open(CONFIG_EXAMPLES_CPU_RST_LTE_DEVPATH, O_RDWR);
	if (fd_EC20_CPU_RST_NB < 0)
	{
		int errcode = errno;
		printf("EC20: ERROR: Failed to open %s: %d\n",CONFIG_EXAMPLES_CPU_RST_LTE_DEVPATH, errcode);
		return;
	}
	
	fd_EC20_NB_PWR_EN = open(CONFIG_EXAMPLES_LTE_PWR_EN_0_DEVPATH, O_RDWR);
	if (fd_EC20_NB_PWR_EN < 0)
	{
		int errcode = errno;
		printf("EC20: ERROR: Failed to open %s: %d\n",CONFIG_EXAMPLES_LTE_PWR_EN_0_DEVPATH, errcode);
		return;
	}
	fd_EC20_CPU_PWR_LTE = open(CONFIG_EXAMPLES_CPU_PWR_LTE_DEVPATH, O_RDWR);
	if (fd_EC20_CPU_PWR_LTE < 0)
	{
		int errcode = errno;
		printf("EC20: ERROR: Failed to open %s: %d\n",CONFIG_EXAMPLES_CPU_PWR_LTE_DEVPATH, errcode);
		return;
	}
	fd_EC20_LTE_AP_READY = open(CONFIG_EXAMPLES_LTE_AP_READY_DEVPATH, O_RDWR);
	if (fd_EC20_LTE_AP_READY < 0)
	{
		int errcode = errno;
		printf("EC20: ERROR: Failed to open %s: %d\n",CONFIG_EXAMPLES_LTE_AP_READY_DEVPATH, errcode);
		return;
	}
	fd_EC20_LTE_DTR = open(CONFIG_EXAMPLES_LTE_DTR_DEVPATH, O_RDWR);
	if (fd_EC20_LTE_DTR < 0)
	{
		int errcode = errno;
		printf("EC20: ERROR: Failed to open %s: %d\n",CONFIG_EXAMPLES_LTE_DTR_DEVPATH, errcode);
		return;
	}
}


void EC20_USART_Init()
{
	fd_EC20_USART = open(CONFIG_EXAMPLES_LTE_EC20_DEVPATH, O_RDWR | O_NOCTTY | O_NONBLOCK | O_NDELAY);
	if (fd_EC20_USART < 0)
	{
		int errcode = errno;
		printf("EC20: ERROR: Failed to open %s: %d\n",CONFIG_EXAMPLES_LTE_EC20_DEVPATH, errcode);
		return;
	}
	printf("%s open\n",CONFIG_EXAMPLES_LTE_EC20_DEVPATH);
	fd_EC20_USART_GPIO_INT = open(CONFIG_EXAMPLES_GPIO_LTE_UART_INT_DEVPATH, O_RDWR);
	if (fd_EC20_USART_GPIO_INT < 0)
	{
		int errcode = errno;
		printf("EC20: ERROR: Failed to open %s: %d\n",CONFIG_EXAMPLES_GPIO_LTE_UART_INT_DEVPATH, errcode);
		return;
	}
	
}
void EC20_power_on(void)
{
	check_ack_timeout = 10;
	while(EC20_send_cmd("AT","OK",1)&&check_ack_timeout)
	{
		if(check_ack_timeout)
		{
			check_ack_timeout--;
			printf("EC20 等待模块上电 \r\n");
		}
		delay_ms(1000);
	}

}

int EC20(int argc, char *argv[])
{
	EC20_GPIO_Init();
	EC20_USART_Init();
	
	GPIO_write(&fd_EC20_NB_PWR_EN, 0);
	delay_ms(1000);
	GPIO_write(&fd_EC20_LTE_DTR,0);

	GPIO_write(&fd_EC20_CPU_RST_NB, 0);
	delay_ms(200);
	GPIO_write(&fd_EC20_CPU_RST_NB, 1);
	delay_ms(200);
	GPIO_write(&fd_EC20_CPU_RST_NB, 0);
	delay_ms(1000);
	

	GPIO_write(&fd_EC20_CPU_PWR_LTE, 0);
	delay_ms(3000);
	GPIO_write(&fd_EC20_CPU_PWR_LTE, 1);
	delay_ms(100);
	GPIO_write(&fd_EC20_CPU_PWR_LTE, 0);

	EC20_power_on();
}

