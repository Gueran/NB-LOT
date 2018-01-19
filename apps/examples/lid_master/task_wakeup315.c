#include <sys/ioctl.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sched.h>
#include <errno.h>
#include <nuttx/leds/userled.h>
#include <nuttx/board.h>
#include <sys/boardctl.h>
#include <nuttx/analog/adc.h>
#include <nuttx/analog/ioctl.h>
#include <nuttx/board.h>
#include <sys/boardctl.h>
#include <errno.h>
#include <nuttx/ioexpander/gpio.h>

#include "task_485.h"
#include "task_wakeup315.h"
#include "task_monitor.h"

/****************************************************************************
 * Private Data
 ****************************************************************************/
static bool  g_wakeup315_started;
static	int  fd_wakeup;
int  wakeup_signal_flag = 0;


/******************************************************************************************************************************
函数名称：void	Wakeup_openLock(struct msg485 *msg485)
功    能：无线唤醒，请求开锁
输入参数：struct msg485
输入参数：无
编写时间：2017.10.09
编 写 人：liushuhe
*******************************************************************************************************************************/
void	Wakeup_openLock(struct msg485 *msg485)
{
	
	if(!pthread_mutex_trylock(&g_485Mutex))
	{
		printf("Wakeup_openLock...........................................................\n");
		msg485->type 	=	OPEN_LOCK;
		pthread_mutex_unlock(&g_485Mutex);
	}
	else
	{
		printf("Wakeup get g_485Mutex Lock fail!\n");
	}
}


/****************************************************************************
 * wakeup_315 无线遥控唤醒，请求开锁
 * liushuhe
 * 2017.10.09
 ****************************************************************************/
void wakeup315_action(int signo,siginfo_t *siginfo, void *arg)
{
	static int cnt; 
	if (signo == SIGUSR1)
	{
		printf("%2d SIGUSR1 received\n",cnt++);
		wakeup_signal_flag = 1;
	}
}


/****************************************************************************
 * wakeup_315
 * liushuhe
 * 2017.10.09
 ****************************************************************************/
int wakeup_315(int argc, char *argv[])
{
	enum gpio_pintype_e pintype;
	struct sigaction act;
	struct sigaction oldact;
	int ret;
	int status;

	
	g_wakeup315_started = true;

	fd_wakeup = open(CONFIG_EXAMPLES_WKUP315_DEVPATH, O_RDONLY);
	if (fd_wakeup < 0)
	{
		printf("slave_wakeup: open %s failed: %d\n", CONFIG_EXAMPLES_WKUP315_DEVPATH, errno);
		goto errout;
	}
	ret = ioctl(fd_wakeup, GPIOC_PINTYPE, (unsigned long)((uintptr_t)&pintype));
	if (ret < 0)
	{
		int errcode = errno;
		fprintf(stderr, "ERROR: Failed to read pintype from %s: %d\n", CONFIG_EXAMPLES_WKUP315_DEVPATH, errcode);
		close(fd_wakeup);
		return EXIT_FAILURE;
	}

	//signal
	memset(&act, 0, sizeof(struct sigaction));
	act.sa_sigaction = wakeup315_action;
	act.sa_flags     = SA_SIGINFO;

	(void)sigemptyset(&act.sa_mask);

	status = sigaction(SIGUSR1, &act, &oldact);
	if (status != 0)
	{
		fprintf(stderr, "Failed to install SIGUSR1 handler, errno=%d\n",errno);
		exit(2);
	}
	
	// Set up to receive signal 
	if(pintype == GPIO_INTERRUPT_PIN)
	{	
		ret = ioctl(fd_wakeup, GPIOC_REGISTER, (unsigned long)SIGUSR1);
		if (ret < 0)
		{
			int errcode = errno;
			fprintf(stderr, "ERROR: Failed to setup for signal from %s: %d\n", CONFIG_EXAMPLES_WKUP315_DEVPATH, errcode);
			close(fd_wakeup);
			return EXIT_FAILURE;
		}
	}
	while(1)
	{
		usleep(200*1000);
		if(wakeup_signal_flag == 1)
		{
			wakeup_signal_flag = 0;
			buzz_alarm();
			bluetooth_init();	
			if(!pthread_mutex_trylock(&g_MonitorMutex))
			{
				printf("Wakeup get g_MonitorMutex Lock ok!\n");
				Wakeup_openLock(&Msg485Data);
				pthread_mutex_unlock(&g_MonitorMutex);
			}
			else
			{
				printf("Wakeup get g_MonitorMutex Lock fail!\n");
			}
			
		}
	}
 errout:
  g_wakeup315_started = false;

  printf("wakeup315: Terminating\n");

  return EXIT_FAILURE;
}






