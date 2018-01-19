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

#include "485.h"


#define COMM_TRM_HEAD 0x68
#define COMM_TRM_FIXED 0x45
#define COMM_TRM_TAIL 0x16

#define MAX_LEN_COMM_TRM_DATA 128

UInt8 SRB = 0;
/**
* @brief  Status of received communication frame
*/
typedef enum
{
    STATUS_IDLE = (UInt8)0,
    STATUS_HEAD,    /* Rx Head=0x68 */
    STATUS_FIXED,   /* Rx FIXED=0x45 */
    STATUS_TYPE,    /* Rx Type FG*/
    STATUS_CMD0,    /* */
    STATUS_CMD1,    /* */
    STATUS_GUID,    /* */
    STATUS_CON0,    /* */
    STATUS_CON1,    /* */
    STATUS_DATA_LEN,/* */
    STATUS_DATA,    /* Data filed */
    STATUS_CS,      /* Rx Len == DATA_LEN*/
    STATUS_TAIL,    /* Tail=0x16 */
    STATUS_END,     /* End of this frame */
} COMM_TRM_STATUS_TypeDef;

/**
* @brief  Data object for received communication frame
*/
typedef struct
{
    UInt8    byCnt; /* Count of 1 field */
    UInt8    byDataLen; /* Length of data field */
    UInt8    byFrameLen; /* Length of frame */
    COMM_TRM_STATUS_TypeDef    eRxStatus;
    UInt8    a_byRxBuf[MAX_LEN_COMM_TRM_DATA];
    UInt8    FG;
    UInt8    CMD0;
    UInt8    CMD1;
    UInt16   GUID;
    UInt8    GUIDCnt;
    UInt8    CON0;
    UInt8    CON1;
    UInt8    CS;
	UInt8    Date[128];
} COMM_TRM_DATA;



/**

* @brief  Data object for received communication frame.
* @note  Prevent race condition that accessed by both ISR and process.
*/
static COMM_TRM_DATA    s_stComm2TrmData;

static	int  		  fd_485_USART[2];
static	int  		  fd_max;


void _485_USART_Init()
{
	fd_485_USART[0] = open(CONFIG_EXAMPLES_RS485A_DEVPATH, O_RDWR | O_NOCTTY | O_NONBLOCK | O_NDELAY);
	if (fd_485_USART[0] < 0)
	{
		int errcode = errno;
		printf("EC20: ERROR: Failed to open %s: %d\n",CONFIG_EXAMPLES_RS485A_DEVPATH, errcode);
		return;
	}
	printf("%s open\n",CONFIG_EXAMPLES_RS485A_DEVPATH);
	fd_485_USART[1] = open(CONFIG_EXAMPLES_RS485B_DEVPATH, O_RDWR);
	if (fd_485_USART[1] < 0)
	{
		int errcode = errno;
		printf("EC20: ERROR: Failed to open %s: %d\n",CONFIG_EXAMPLES_RS485B_DEVPATH, errcode);
		return;
	}
	printf("%s open\n",CONFIG_EXAMPLES_RS485B_DEVPATH);
	if(fd_485_USART[0] > fd_485_USART[1])
	{
		fd_max = fd_485_USART[0] ;
	}
	else
	{
		fd_max = fd_485_USART[1] ;
	}
}

void ClearCommFrame()
{
    memset(&s_stComm2TrmData,0,sizeof(COMM_TRM_DATA));
}

void process_poll(COMM_TRM_DATA *Comm2TrmData)
{
	int i = 0;
    printf("***************************************\n");
    printf("byCnt     : %d\n",s_stComm2TrmData.byCnt);
    printf("byDataLen : %d\n",s_stComm2TrmData.byDataLen); 
    printf("byFrameLen: %d\n",s_stComm2TrmData.byFrameLen); 
    printf("eRxStatus : %d\n",s_stComm2TrmData.eRxStatus); 
    printf("FG        : 0x%02X\n",s_stComm2TrmData.FG); 
    printf("CMD0      : 0x%02X\n",s_stComm2TrmData.CMD0); 
    printf("CMD1      : 0x%02X\n",s_stComm2TrmData.CMD1); 
    printf("GUID      : 0x%02X\n",s_stComm2TrmData.GUID); 
    printf("CON0      : 0x%02X\n",s_stComm2TrmData.CON0); 
    printf("CON1      : 0x%02X\n",s_stComm2TrmData.CON1); 
    printf("CS        : 0x%02X\n",s_stComm2TrmData.CS);
	printf("Date      :");
	for(i = 0; i<s_stComm2TrmData.byDataLen;i++)
	{
		printf(" 0x%02X\t",s_stComm2TrmData.Date[i]);
	}
	printf("\n");
    printf("***************************************\n");
	if(0x11 == s_stComm2TrmData.CON0)
	{
		short nu = ((s_stComm2TrmData.Date[0]>>4)*1000)+((s_stComm2TrmData.Date[0]&0x0f)*100)+((s_stComm2TrmData.Date[1]>>4)*10)+((s_stComm2TrmData.Date[1]&0x0f)*1);
		printf("%d\n",nu);
		setDeviceInfoLedTemperature(nu);
	}
}

/**
  * @brief  Put a data that received by UART into buffer.
  * @note  Prevent race condition this called by ISR. 
  * @param  uint8_t byData: the data received by UART.
  * @retval  None
  */
void comm2trm_RxUartData(UInt8 byData)
{
    /* Update status according to the received data */
    switch (s_stComm2TrmData.eRxStatus)
    {
        case STATUS_IDLE:
            if (COMM_TRM_HEAD == byData) /* Is Head */
            {
                s_stComm2TrmData.eRxStatus = STATUS_HEAD;
            }
            else
            {
                printf("%d\n",__LINE__);
                goto rx_exception;
            }
            break;
        case STATUS_HEAD:
            if (COMM_TRM_FIXED == byData) /* Is FIXED */
            {
                s_stComm2TrmData.eRxStatus = STATUS_FIXED;
            }
            else
            {
                printf("%d\n",__LINE__);
                goto rx_exception;
            }
            break;
        case STATUS_FIXED:
            if (0x00 == byData) /* Valid type */
            {
                s_stComm2TrmData.eRxStatus = STATUS_TYPE;
                s_stComm2TrmData.FG        = byData;
            }
            else
            {
                printf("%d\n",__LINE__);
                goto rx_exception;
            }
            break;
        case STATUS_TYPE:
            s_stComm2TrmData.eRxStatus = STATUS_CMD0;
            s_stComm2TrmData.CMD0      = byData;
            break;
        case STATUS_CMD0:
            s_stComm2TrmData.eRxStatus = STATUS_CMD1;
            s_stComm2TrmData.CMD1      = byData;
            s_stComm2TrmData.GUIDCnt   = 0;
            break;

        case STATUS_CMD1:
            if(0 == s_stComm2TrmData.GUIDCnt)
            {
                s_stComm2TrmData.GUID = byData;
                s_stComm2TrmData.GUIDCnt++;
            }
            else if(1 == s_stComm2TrmData.GUIDCnt)
            {
                s_stComm2TrmData.GUID = (s_stComm2TrmData.GUID << 8) | byData;
                s_stComm2TrmData.eRxStatus = STATUS_GUID;
            }
            else
            {
                printf("%d\n",__LINE__);
                goto rx_exception;
            }
            break;
        case STATUS_GUID:
            s_stComm2TrmData.eRxStatus = STATUS_CON0;
            s_stComm2TrmData.CON0      = byData;
            break;
        case STATUS_CON0:
            s_stComm2TrmData.eRxStatus = STATUS_CON1;
            s_stComm2TrmData.CON1      = byData;
            break;
        case STATUS_CON1:
            if (byData <= MAX_LEN_COMM_TRM_DATA) /* Valid data size */
            {
                s_stComm2TrmData.eRxStatus = STATUS_DATA;
                s_stComm2TrmData.byDataLen = byData;
                s_stComm2TrmData.byCnt     = 0;
            }
            else
            {
                printf("%d\n",__LINE__);
                goto rx_exception;
            }
            break;
        case STATUS_DATA:
			s_stComm2TrmData.Date[s_stComm2TrmData.byCnt] = byData; 
            ++s_stComm2TrmData.byCnt;
            if (s_stComm2TrmData.byCnt < s_stComm2TrmData.byDataLen)
            {
              
            }
            else
            {
                s_stComm2TrmData.eRxStatus = STATUS_CS;
            }
            break;
        case STATUS_CS:     
            if (1) /* Valid type */
            {
                s_stComm2TrmData.eRxStatus = STATUS_TAIL;
                s_stComm2TrmData.CS        = byData;
            }
            else
            {
                printf("%d 0x%02X\n",__LINE__,byData);
                goto rx_exception;
            }
            break;            
        case STATUS_TAIL:
            if (COMM_TRM_TAIL == byData)
            {
                /* We received a frame of data, now tell process to deal with it! */
                process_poll(&s_stComm2TrmData);
                ClearCommFrame();
            }
            else
            {
                printf("%d 0x%02X\n",__LINE__,byData);
                goto rx_exception;
            }
            break;
        default:
            printf("Error: Bad status of comm2trm_RxUartData().\r\n");
            break;
    }

    /* Save the received data */
    s_stComm2TrmData.a_byRxBuf[s_stComm2TrmData.byFrameLen++] = byData;
    return;

rx_exception:
    ClearCommFrame();
    return; 
}
void getLedTemperature()
{

}
void getBrightness()
{

}
void getBaseTemperature()
{
	int 	iBytes = 0;
	int     i = 0;
	UInt8 senddate[] = {0xfe,0xfe,0xfe
					   ,0x68,0x45
					   ,0x00         //FG=00
					   ,0x20         //CMD0 获取
					   ,0x08			//CMD1 HEX
					   ,0xB0,0x00     //GUID 主控默认A0 前端默认B0 后端 默认C0
					   ,0x11          //读取温度
					   ,0x30          //单位摄氏度
					   ,0x01          //数据域如果没有任何数据L=0X01
					   ,SRB           //SR 帧交互次数
					   ,0xC8
					   ,0x16       //end
					   };
	SRB  = (SRB+1)%16;
	senddate[sizeof(senddate)-2] = 0;
	for(i = 3;i< sizeof(senddate)-2;i++)
	{
		senddate[sizeof(senddate)-2] += senddate[i];
		
	}
	printf("CS:0x%02x\n",senddate[sizeof(senddate)-2]);
	iBytes = write(fd_485_USART[1],senddate,sizeof(senddate));
	printf("fd_485_USARTA Write <%d>Bytes\n",iBytes);
}

void getColorTemperature()
{
		int 	iBytes = 0;
	int     i = 0;
	UInt8 senddate[] = {0xfe,0xfe,0xfe
					   ,0x68,0x45
					   ,0x40         //FG=00
					   ,0x20         //CMD0 获取
					   ,0x08			//CMD1 HEX
					   ,0xC0,0x00     //GUID 主控默认A0 前端默认B0 后端 默认C0
					   ,0x11          //读取色温
					   ,0x30          //单位摄氏度
					   ,0x01          //数据域如果没有任何数据L=0X01
					   ,SRB           //SR 帧交互次数
					   ,0xC8
					   ,0x16       //end
					   };
	SRB  = (SRB+1)%16;
	senddate[sizeof(senddate)-2] = 0;
	for(i = 3;i< sizeof(senddate)-2;i++)
	{
		senddate[sizeof(senddate)-2] += senddate[i];
		
	}
	printf("CS:0x%02x\n",senddate[sizeof(senddate)-2]);
	iBytes = write(fd_485_USART[1],senddate,sizeof(senddate));
	printf("fd_485_USARTA Write <%d>Bytes\n",iBytes);
}
void getVoltage()
{
	int 	iBytes = 0;
	int     i = 0;
	
	UInt8 senddate[] = {0xfe,0xfe,0xfe
					   ,0x68,0x45
					   ,0x40         //FG=00
					   ,0x20         //CMD0 获取
					   ,0x08			//CMD1 HEX
					   ,0xC0,0x00     //GUID 主控默认A0 前端默认B0 后端 默认C0
					   ,0x13          //读取电压
					   ,0x10          //单位0x01V
					   ,0x01          //数据域如果没有任何数据L=0X01
					   ,SRB           //SR 帧交互次数
					   ,0xC8
					   ,0x16       //end
					   };
	SRB  = (SRB+1)%16;
	senddate[sizeof(senddate)-2] = 0;
	for(i = 3;i< sizeof(senddate)-2;i++)
	{
		senddate[sizeof(senddate)-2] += senddate[i];
		
	}
	for(i = 0;i < sizeof(senddate);i++)
	{
		printf("%02X\t",senddate[i]);
	}
	printf("\n");
	printf("CS:0x%02x\n",senddate[sizeof(senddate)-2]);
	iBytes = write(fd_485_USART[1],senddate,sizeof(senddate));
	printf("fd_485_USARTA Write <%d>Bytes\n",iBytes);
}
void getCurrent()
{
	int 	iBytes = 0;
	int     i = 0;
	UInt8 senddate[] = {0xfe,0xfe,0xfe
					   ,0x68,0x45
					   ,0x40         //FG=40
					   ,0x20         //CMD0 获取
					   ,0x08			//CMD1 HEX
					   ,0xC0,0x00     //GUID 主控默认A0 前端默认B0 后端 默认C0
					   ,0x14          //读取电流
					   ,0x20          //单位0.01A
					   ,0x01          //数据域如果没有任何数据L=0X01
					   ,SRB           //SR 帧交互次数
					   ,0x00
					   ,0x16       //end
					   };
	SRB  = (SRB+1)%16;
	senddate[sizeof(senddate)-2] = 0;
	for(i = 3;i< sizeof(senddate)-2;i++)
	{
		senddate[sizeof(senddate)-2] += senddate[i];
		
	}
	printf("CS:0x%02x\n",senddate[sizeof(senddate)-2]);
	iBytes = write(fd_485_USART[1],senddate,sizeof(senddate));
	printf("fd_485_USARTA Write <%d>Bytes\n",iBytes);
}


void setMainLEDSwitch()
{
	int 	iBytes = 0;
	int     i = 0;
	UInt8 senddate[] = {0xfe,0xfe,0xfe
					   ,0x68,0x45
					   ,0x40         //FG=00
					   ,0x40         //CMD0 下发
					   ,0x08			//CMD1 HEX
					   ,0xC0,0x00     //GUID 主控默认A0 前端默认B0 后端 默认C0
					   ,0x42          //主灯关
					   ,0x00          //单位 
					   ,0x01          //数据域如果没有任何数据L=0X01
					   ,SRB           //SR 帧交互次数
					   ,0xC8
					   ,0x16       //end
					   };
	SRB  = (SRB+1)%16;
	senddate[sizeof(senddate)-2] = 0;
	for(i = 3;i< sizeof(senddate)-2;i++)
	{
		senddate[sizeof(senddate)-2] += senddate[i];
		
	}
	printf("CS:0x%02x\n",senddate[sizeof(senddate)-2]);
	iBytes = write(fd_485_USART[1],senddate,sizeof(senddate));
	printf("fd_485_USARTA Write <%d>Bytes\n",iBytes);

}
void setBLEDSwitch()
{
	int 	iBytes = 0;
	int     i = 0;
	UInt8 senddate[] = {0xfe,0xfe,0xfe
					   ,0x68,0x45
					   ,0x40         //FG=00
					   ,0x40         //CMD0 下发
					   ,0x08			//CMD1 HEX
					   ,0xC0,0x00     //GUID 主控默认A0 前端默认B0 后端 默认C0
					   ,0x45          //辅灯关
					   ,0x00          //单位 
					   ,0x01          //数据域如果没有任何数据L=0X01
					   ,SRB           //SR 帧交互次数
					   ,0xC8
					   ,0x16       //end
					   };
	SRB  = (SRB+1)%16;
	senddate[sizeof(senddate)-2] = 0;
	for(i = 3;i< sizeof(senddate)-2;i++)
	{
		senddate[sizeof(senddate)-2] += senddate[i];
		
	}
	printf("CS:0x%02x\n",senddate[sizeof(senddate)-2]);
	iBytes = write(fd_485_USART[1],senddate,sizeof(senddate));
	printf("fd_485_USARTA Write <%d>Bytes\n",iBytes);

}


int _485(int argc, char *argv[])
{
	fd_set 	rfds;	
	int  	iRet = 0;
	int  	iBytes = 0;
    int     cnt = 0;
	int     i;
	struct timeval timeout;
	char 	cArray[256];
	_485_USART_Init();

	while(1)
	{
		
		UInt8 ondate[] = {0xfe,0xfe,0xfe,0x68 ,0x45 ,0x00 ,0x20 ,0x08 ,0xC0 ,0x00 ,0x13 ,0x10 ,0x01 ,0x01 ,0xff ,0x16};
		usleep(5000*1000L);
		getBaseTemperature();
		
		//getVoltage();
		//getCurrent();
		//setBLEDSwitch();
		FD_ZERO(&rfds);											
		FD_SET(fd_485_USART[0], &rfds);									
		FD_SET(fd_485_USART[1], &rfds);
		timeout.tv_sec = 5;
		timeout.tv_usec = 0;

		iRet = select(fd_max+1, &rfds, NULL, NULL, &timeout);  	//recv-timeout

		if (iRet < 0) 
		{
			printf("select error!!!\n");
		}
		else if(iRet == 0)
		{
			printf("485 rcv timeout!!!\n");
		}
		else
		{
			if(FD_ISSET(fd_485_USART[0], &rfds)) 
			{
				usleep(100*1000L);                                     //sleep 100ms
				memset(cArray, '\0', sizeof(cArray));
				iBytes = read(fd_485_USART[0], cArray, sizeof(cArray));
			    tcflush(fd_485_USART[0], TCIFLUSH);

				printf("fd_485_USARTA  <%d>Read <%d>Bytes Data:%s\n",cnt++,iBytes,cArray);

				for(i = 0; i < iBytes;i++)
			    {
			        comm2trm_RxUartData(cArray[i]);
			    }
				
			}
			else if(FD_ISSET(fd_485_USART[1], &rfds))
			{
				usleep(100*1000L);                                     //sleep 100ms
				memset(cArray, '\0', sizeof(cArray));
				iBytes = read(fd_485_USART[1], cArray, sizeof(cArray));
			    tcflush(fd_485_USART[1], TCIFLUSH);

				printf("fd_485_USARTB <%d>Read <%d>Bytes Data:%s\n",cnt++,iBytes,cArray);
				for(i = 0; i < iBytes;i++)
			    {
			    	//printf("0x%02X \t",cArray[i]);
			        comm2trm_RxUartData(cArray[i]);
			    }
			
			}
		}
	}
}

