#include <sys/ioctl.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sched.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>

#include <nuttx/analog/adc.h>
#include <nuttx/analog/ioctl.h>
#include <nuttx/leds/userled.h>
#include <nuttx/board.h>
#include <sys/boardctl.h>

#include <nuttx/board.h>
#include <sys/boardctl.h>
#include <errno.h>
#include <nuttx/ioexpander/gpio.h>

#include "task_adc.h"
#include "task_flash.h"
#include "task_485.h"
#include "task_gprs.h"
#include "task_bluetooth.h"
#include "task_monitor.h"
#include "task_motor.h"

/*****************************************************************************************************************************
 * Private Data
 ****************************************************************************************************************************/
pthread_mutex_t g_TimIntMutex		= PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t g_MonitorMutex		= PTHREAD_MUTEX_INITIALIZER;

static		bool  		g_monitor_started;

char		TimeInt_SampleFlag = 0;


struct  	TimeStruct  DisLocalTime;
struct		tm 			*tmp;
struct 		rtc_time	rtctime;
time_t		timelocal;

/****************************************************************************
 * getSystime
 * liushuhe
 * 2017.10.11
 ****************************************************************************/
void  getSystime(void)
{
	time(&timelocal);	
	tmp = localtime(&timelocal);  //获取本地时间


	DisLocalTime.Year		=	1900+tmp->tm_year;
	DisLocalTime.Month		=	tmp->tm_mon + 1;
	DisLocalTime.Day		=	tmp->tm_mday;
	DisLocalTime.Hour		=	tmp->tm_hour;
	DisLocalTime.Minute	=	tmp->tm_min;
	DisLocalTime.Second	=	tmp->tm_sec;
}
/****************************************************************************
 * setSystime
 * liushuhe
 * 2017.10.11
 ****************************************************************************/
int setSystime(struct rtc_time *rtc)
{
	//struct rtc_time rtc;  
	struct tm _tm;  
	struct timeval tv;  
	time_t timep;  

	_tm.tm_sec 	= rtc->tm_sec;  
	_tm.tm_min 	= rtc->tm_min;  
	_tm.tm_hour = rtc->tm_hour;  
	_tm.tm_mday = rtc->tm_mday;  
	_tm.tm_mon 	= rtc->tm_mon - 1;  
	_tm.tm_year = rtc->tm_year - 1900;  

	timep = mktime(&_tm);  
	tv.tv_sec = timep;  
	tv.tv_usec = 0;  
	if(settimeofday (&tv, (struct timezone *) 0) < 0)  
	{  
		printf("Set system datatime error!/n");  
		return -1;  
	}  
	return 0;  

}
/****************************************************************************
 * setRtcTime
 * liushuhe
 * 2017.10.18
 ****************************************************************************/
int setRtcTime(int fd,struct rtc_time *rtc)
{
	int ret;
	ret = ioctl(fd, RTC_SET_TIME,(unsigned long)((uintptr_t)rtc));
	if (ret < 0)
	{
		int errcode = errno;
		printf("getRtcTime: ERROR: ioctl(ULEDIOC_SUPPORTED) failed: %d\n",errcode);
		return -1;
	}
	return 1;

}
/****************************************************************************
 * getRtcTime
 * liushuhe
 * 2017.10.18
 ****************************************************************************/
int getRtcTime(int fd,struct rtc_time *rtc)
{
	int ret;
	ret = ioctl(fd, RTC_RD_TIME,(unsigned long)((uintptr_t)rtc));
	if (ret < 0)
	{
		int errcode = errno;
		printf("getRtcTime: ERROR: ioctl(ULEDIOC_SUPPORTED) failed: %d\n",errcode);
		return -1;
	}

	//year + 1900
	rtc->tm_year = rtc->tm_year + 1900;
	//tm_mon + 1
	rtc->tm_mon = rtc->tm_mon + 1;
	
	return 1;
}


int master_monitor(int argc, char *argv[])
{
	int fd_gprs_copy;
	int fd_rtc;
	g_monitor_started = false;

	int ret; 
	fd_gprs_copy = open(CONFIG_EXAMPLES_GPRS_DEVPATH, O_RDWR | O_NOCTTY | O_NONBLOCK | O_NDELAY);
	if (fd_gprs_copy < 0)
	{
		int errcode = errno;
		printf("gprs: ERROR: Failed to open %s: %d\n",CONFIG_EXAMPLES_GPRS_DEVPATH, errcode);
	}

	ret = gprs_rst(fd_gprs_copy,&GprsData);
	if(ret == SUCCESS)
	{
		printf("GPRS OK\n");
	}
	else
	{
		printf("GPRS ERROR\n");
	}


   return 1;

}


