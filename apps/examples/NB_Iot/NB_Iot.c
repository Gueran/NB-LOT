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
#include <ctype.h>

#include "GPIO.h"
#include "NB_Iot.h"




static	int  		  fd_NB_Iot_USART;
static	int  		  fd_NB_Iot_USART_GPIO_INT;

static	int  		  fd_NB_Iot_CPU_RST_NB;
static	int  		  fd_NB_Iot_NB_PWR_EN;



static	int  		  fd_stdin;
static	int  		  fd_max;
static  UInt8 check_ack_timeout = 10;
static  UInt8 ue_exist_flag = 0;
static  UInt8 ue_need_reboot_flag = 0;
static char NB_Iot_rcvd_buf[256];

struct  NB_Iot_init
{
	char  InitOK;
	char  State;
	char  ATcmd_num;
};

//字节流转换为十六进制字符串  
void ByteToHexStr(const unsigned char* source, char* dest, int sourceLen)  
{  
    short i;  
    unsigned char highByte, lowByte;  
  	printf("sourceLen:%d\n",sourceLen);
    for (i = 0; i < sourceLen; i++)  
    {  
        highByte = source[i] >> 4;  
        lowByte = source[i] & 0x0f ;  
  
        highByte += 0x30;  
  
        if (highByte > 0x39)  
                dest[i * 2] = highByte + 0x07;  
        else  
                dest[i * 2] = highByte;  
  
        lowByte += 0x30;  
        if (lowByte > 0x39)  
            dest[i * 2 + 1] = lowByte + 0x07;  
        else  
            dest[i * 2 + 1] = lowByte;  
    } 
	dest[i * 2] = '\0';
	printf("dest:%d\n",strlen(dest));
    return ;  
} 
//十六进制字符串转换为字节流  
void HexStrToByte(const char* source, unsigned char* dest, int sourceLen)  
{  
    short i;  
    unsigned char highByte, lowByte;  
      
    for (i = 0; i < sourceLen; i += 2)  
    {  
        highByte = toupper(source[i]);  
        lowByte  = toupper(source[i + 1]);  
  
        if (highByte > 0x39)  
            highByte -= 0x37;  
        else  
            highByte -= 0x30;  
  
        if (lowByte > 0x39)  
            lowByte -= 0x37;  
        else  
            lowByte -= 0x30;  
  
        dest[i / 2] = (highByte << 4) | lowByte;  
    }  
    return ;  
}  


/****************************************************************************
 * Name: NB_Iot
 ****************************************************************************/
static void delay_ms(UInt16 ms)
{				  
	usleep(ms * 1000L);
} 

void NB_Iot_USART_Init()
{
	fd_NB_Iot_USART = open(CONFIG_EXAMPLES_NB_IOT_DEVPATH, O_RDWR | O_NOCTTY | O_NONBLOCK | O_NDELAY);
	if (fd_NB_Iot_USART < 0)
	{
		int errcode = errno;
		printf("NB_Iot: ERROR: Failed to open %s: %d\n",CONFIG_EXAMPLES_NB_IOT_DEVPATH, errcode);
		return;
	}
	printf("%s open\n",CONFIG_EXAMPLES_NB_IOT_DEVPATH);
	fd_NB_Iot_USART_GPIO_INT = open(CONFIG_EXAMPLES_GPIO_NB_UART_INT_DEVPATH, O_RDWR);
	if (fd_NB_Iot_USART_GPIO_INT < 0)
	{
		int errcode = errno;
		printf("NB_Iot: ERROR: Failed to open %s: %d\n",CONFIG_EXAMPLES_GPIO_NB_UART_INT_DEVPATH, errcode);
		return;
	}
	
}
void NB_Iot_GPIO_Init()
{
	fd_NB_Iot_CPU_RST_NB = open(CONFIG_EXAMPLES_CPU_RST_NB_DEVPATH, O_RDWR);
	if (fd_NB_Iot_CPU_RST_NB < 0)
	{
		int errcode = errno;
		printf("NB_Iot: ERROR: Failed to open %s: %d\n",CONFIG_EXAMPLES_CPU_RST_NB_DEVPATH, errcode);
		return;
	}
	fd_NB_Iot_NB_PWR_EN = open(CONFIG_EXAMPLES_NB_PWR_EN_0_DEVPATH, O_RDWR);
	if (fd_NB_Iot_NB_PWR_EN < 0)
	{
		int errcode = errno;
		printf("NB_Iot: ERROR: Failed to open %s: %d\n",CONFIG_EXAMPLES_NB_PWR_EN_0_DEVPATH, errcode);
		return;
	}
}

//检查返回的响应是否符合预期
//传入参数为预期返回的字符串
//返回0，为检测到预期值
//其他值，预期字符所在的位置
UInt8* NB_Iot_check_ack(char *str)
{
	char *strx=0;
	memset(NB_Iot_rcvd_buf,0,sizeof(NB_Iot_rcvd_buf));
	read(fd_NB_Iot_USART, NB_Iot_rcvd_buf, sizeof(NB_Iot_rcvd_buf));
	tcflush(fd_NB_Iot_USART, TCIFLUSH);
	printf("recv:|%s|\n",NB_Iot_rcvd_buf);
	strx=strstr((const char*)NB_Iot_rcvd_buf,(const char*)str);
	return (UInt8*)strx;
}

//发生at指令函数
//cmd:at指令，ack：预期响应，waittime,超时时间
//返回0，发送成功
//返回1，发送超时
UInt8 NB_Iot_send_cmd(char *cmd,char *ack,uint16_t waittime)
{
	UInt8 res=0; 
	fd_set 	rfds;
	struct timeval timeout;
	int  	iRet = 0;
	char  sendcmd[600];
	//int   sendlen = 0;
	//int   sendcnt = 0;
	//int   sendOk = 0;
	//int   count = 0;

	
	//sprintf(sendcmd,"%s\r\n",cmd);

	//printf("|%s| \r\n",sendcmd);

	//sendlen = strlen(sendcmd);
#if 0
	while(sendlen != sendOk)
	{
		if((sendlen-sendOk)>128)
		{
			sendcnt = 128;
		}
		else
		{
			sendcnt = sendlen-sendOk;
		}
		count = write(fd_NB_Iot_USART,sendcmd[sendOk],sendcnt);
		printf("sendlen:%d sendOk:%d count:%d\n",sendlen,sendOk,count);
		sendOk  = sendOk+count;
		delay_ms(50);
	}
#endif
	sprintf(sendcmd,"%s\r\n",cmd);

	printf("|%s| \r\n",sendcmd);

	printf("send:%d\n",write(fd_NB_Iot_USART,sendcmd,strlen(sendcmd)));
	
	//delay_ms(100);
	if(ack&&waittime)
	{
			FD_ZERO(&rfds);											
			FD_SET(fd_NB_Iot_USART, &rfds);									
			timeout.tv_sec = waittime;
			timeout.tv_usec = 0;
			iRet = select(fd_NB_Iot_USART+1, &rfds, NULL, NULL, &timeout);  	//recv-timeout
			if (iRet < 0) 
			{
				printf("select error!!!\n");
				return 1;
			}
			else if(iRet == 0)
			{
				printf("NB_Iot_dev rcv timeout!!!\n");
				return 1;	

			}
			else
			{
				if(FD_ISSET(fd_NB_Iot_USART, &rfds)) 
				{
					delay_ms(100);
					if(NB_Iot_check_ack(ack))return 0;
				}
			}
			
	}
	return 1;	
} 

//上电程序，检测模块是否连接，检查配置是否为自动模式，是否为需要的频段

void BC95_power_on(void)
{
	//BC95_send_cmd(SET_UE_REBOOT,"REBOOT",100);
	check_ack_timeout = 10;
	ue_exist_flag = 1;
	
	while(NB_Iot_send_cmd("AT","OK",5)&&check_ack_timeout)
	{
	
		if(check_ack_timeout)
		{
			check_ack_timeout--;
			ue_exist_flag = 0;
			printf("等待模块上电 \r\n");
		}
		delay_ms(1000);
	}
	//NB_Iot_send_cmd(QUERY_UE_BAND,SET_UE_BAND_5,1);
	//判断模块是否是自动连接模式，如果不是则将模块设置成自动模式
	if(ue_exist_flag&&!NB_Iot_send_cmd(QUERY_UE_CONNECT_MODE,"AUTOCONNECT,FALSE",1))
	{
		check_ack_timeout = 3;
		while(check_ack_timeout)
		{
			check_ack_timeout--;
			if(NB_Iot_send_cmd(SET_UE_AUTOCONNECT,"OK",1))
			{
				printf("设置为自动模式成功！\r\n");
				break;
			}
			delay_ms(1000);
		}
		ue_need_reboot_flag =1;
	}
	//判断模块是否是默认设置频段，如果不是则设置成默认频段
	if(ue_exist_flag&&NB_Iot_send_cmd(QUERY_UE_BAND,UE_DEFAULT_BAND,1))
	{
		NB_Iot_send_cmd(SET_UE_DEFAULT_BAND,UE_DEFAULT_BAND,1);
		printf("设置默认频段！\r\n");
		ue_need_reboot_flag = 1;
	}
	//重启模块生效配置
	if(ue_exist_flag&&ue_need_reboot_flag)
	{
		ue_need_reboot_flag = 0;
		check_ack_timeout = 10;
		NB_Iot_send_cmd(SET_UE_REBOOT,"REBOOT",1);
		printf("重启模块！\r\n");
		while(check_ack_timeout&&!NB_Iot_check_ack("Neul"))
		{
			if(NB_Iot_check_ack("Neul"))
			{
				break;
			}else
			{
				check_ack_timeout--;
				delay_ms(1000);
			}
		}
	}
}



//检查模块的网络状态，检测器件LED1会闪烁，LED1常亮为附网注网成功
//此函数不检查联网状态，仅检查附网注网状态，联网状态可以使用BC95_send_cmd，单独检测
//附网注网失败或者超时返回0，返回1附网注网成功，返回2附网成功
UInt8 query_net_status(void)
{
	UInt8 res = 0;
	UInt8 attached_flag = 0;
	UInt8 registered_flag = 0;
	check_ack_timeout = 20;
	
	while(!(attached_flag&&registered_flag)&&check_ack_timeout)
	{
		if(!NB_Iot_send_cmd(QUERY_UE_SCCON_STATS,UE_CONNECTED,1))
		{
			attached_flag = 1;
			registered_flag = 1;
			res = 1;
			printf("附网、注网成功！\r\n");
			break;
		}else
		{
			if(!attached_flag)
			{
				if(!NB_Iot_send_cmd(QUERY_UE_ATTACH_STATS,UE_ATTACHED_STATS,1))
				{
					printf("附网成功!\r\n");
					attached_flag = 1;
					res =2;
				}else
				{
					printf("正在附网...\r\n");

					attached_flag = 0;
				}
			}
			if(attached_flag&&!registered_flag)
			{
				if(attached_flag&&!NB_Iot_send_cmd(QUERY_UE_EREG_STATS,UE_EREGISTERED_STATS,1))
				{
					printf("注网成功！\r\n");
					registered_flag = 1;
					res =1;
				}else
				{
					printf("正在注网...\r\n");
					registered_flag = 0;
				}
			}				
		}
		check_ack_timeout--;
		delay_ms(500);
		if(!check_ack_timeout&&!attached_flag&&!registered_flag)
		{
			printf("附网、注网失败！\r\n");
		}
	}
	return res;
}

//字符串转整形，stm32不支持标准的atoi,这里自己实现
int myatoi(const char *str)
{
	int s=0;
	uint8_t falg=0;
	
	while(*str==' ')
	{
		str++;
	}

	if(*str=='-'||*str=='+')
	{
		if(*str=='-')
		falg=1;
		str++;
	}

	while(*str>='0'&&*str<='9')
	{
		s=s*10+*str-'0';
		str++;
		if(s<0)
		{
			s=2147483647;
			break;
		}
	}
	return s*(falg?-1:1);
}

//读取数据，截取接收缓存中所需的数据保存到des,pos为起始地址，len为截取长度
void get_str_data(char* des,char pos,char len)
{
	printf("1%s\r\n",NB_Iot_rcvd_buf);
	memcpy(des,NB_Iot_rcvd_buf+pos,len);
}	

//创建UDP链接，传入本地UDP端口号，返回0-6的socket id号，
UInt8 creat_UDP_socket(char* local_port)
{
	char data[10]="";
	UInt8 socket_id = 7;
	char temp[64]="AT+NSOCR=DGRAM,17,";
	strcat(temp,local_port);
	strcat(temp,",1");
	if(!NB_Iot_send_cmd(temp,"OK",100))
	{
		get_str_data(data,2,1);
		socket_id = (UInt8)myatoi(data);
		printf("Socket创建成功，句柄ID--> %d！\r\n",socket_id);
		return socket_id;
	}
	printf("Socket创建失败，已经创建或端口被占用！\r\n");
	return socket_id;
}
//发送数据函数，传入socket,主机IP，远程主机端口，数据长度，数据
//这里暂时使用字符串参数
//返回值0，发送成功（鉴于UDP为报文传输，数据主机是否接收到模块是无法确认的）
//返回值1，发送失败
uint8_t send_UDP_msg(char *socket,char *hostIP,char *port,char *dataLen,char *data)
{
	char ptr[600]="AT+NSOST=";
	strcat(ptr,socket);
	strcat(ptr,",");
	strcat(ptr,hostIP);
	strcat(ptr,",");
	strcat(ptr,port);
	strcat(ptr,",");
	strcat(ptr,dataLen);
	strcat(ptr,",");
	strcat(ptr,data);
	if(!NB_Iot_send_cmd(ptr,"OK",200))
	{
		printf("发送数据--> %s！\r\n",ptr);	
		return 0;
	}
	return 1;
}

int NB_Iot(int argc, char *argv[])
{
	char date[256];
	char senddate[512];
	char sendlen[32];
	int 	iBytes = 0;
	char *strx=0;
	char 	cArray[256];
	int  resultcount = 0;
	fd_set 	rfds;
	int  	iRet = 0;
	char *result = NULL;
	struct timeval timeout;
	timeout.tv_sec =  0;
	timeout.tv_usec = 0;
	NB_Iot_GPIO_Init();
	NB_Iot_USART_Init();
	GPIO_write(&fd_NB_Iot_NB_PWR_EN, 0);
	delay_ms(1000);
	GPIO_write(&fd_NB_Iot_CPU_RST_NB, 0);
	delay_ms(200);
	GPIO_write(&fd_NB_Iot_CPU_RST_NB, 1);
	delay_ms(200);
	GPIO_write(&fd_NB_Iot_CPU_RST_NB, 0);
	delay_ms(1000);
#ifdef BC95_PWR_ON_TEST
	BC95_power_on();
#endif	
	if(query_net_status())
	{
		creat_UDP_socket(UE_LOCAL_UDP_PORT);
		while(1)
		{
			if(timeout.tv_sec == 0)
			{
			memset(date,0,sizeof(date));
			memset(senddate,0,sizeof(senddate));
			DeviveInfotoJson(date);
			ByteToHexStr(date,senddate,strlen(date));
			printf("sendlen:%d\n",strlen(senddate));
			sprintf(sendlen,"%d",strlen(date));	
			printf("sendlen:%s\n",sendlen);
			send_UDP_msg("0",SERVER_HOST_UDP_IP,SERVER_HOST_UDP_PORT,sendlen,senddate);
			timeout.tv_sec = 10;
			timeout.tv_usec = 0;
			}
			FD_ZERO(&rfds);											
			FD_SET(fd_NB_Iot_USART, &rfds);
			iRet = select(fd_NB_Iot_USART+1, &rfds, NULL, NULL, &timeout);  	//recv-timeout
			if (iRet < 0) 
			{
				printf("select error!!!\n");
			}
			else if(iRet == 0)
			{
				timeout.tv_sec =  0;
				timeout.tv_usec = 0;
			}
			else
			{
				if(FD_ISSET(fd_NB_Iot_USART, &rfds))
				{
					usleep(100*1000L);                                     //sleep 100ms
					memset(cArray, '\0', sizeof(cArray));
					iBytes = read(fd_NB_Iot_USART, cArray, sizeof(cArray));
				    tcflush(fd_NB_Iot_USART, TCIFLUSH);
					printf("recv:|%s|\n",cArray);
					strx=strstr((const char*)cArray,(const char*)"+NSONMI:0,");
					if(strx)
					{
						char data[256] = "";
						char temp[64]="AT+NSORF=";
						sprintf(temp,"AT+NSORF=%s",&cArray[10]);
						if(!NB_Iot_send_cmd(temp,"OK",100))
						{
							get_str_data(data,0,256);
							resultcount = 0;
							
							result = strtok(data,",");
							while(result!=NULL)
							{
								resultcount++;
								result = strtok(NULL,"," );
								printf("%s\r\n",result);
							}
							
							
						}
					}
					else
					{

					}	
				}
			}
		}
	}
  printf("gprs: Terminating\n");
  return EXIT_FAILURE;
}

