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
#include "task_monitor.h"

/****************************************************************************
 * Private Data
 ****************************************************************************/
static bool  g_close315_started;
static	int  fd_close;
int  close_signal_flag = 0;


/******************************************************************************************************************************
�������ƣ�void	Wakeup_closeLock(struct msg485 *msg485)
��    �ܣ����߻��ѣ��������
���������struct msg485
�����������
��дʱ�䣺2017.10.09
�� д �ˣ�liushuhe
*******************************************************************************************************************************/
void	Wakeup_closeLock(struct msg485 *msg485)
{
	
	if(!pthread_mutex_trylock(&g_485Mutex))
	{
		printf("close Lock......................................................\n");
		msg485->type 	=	CLOSE_LOCK;
		pthread_mutex_unlock(&g_485Mutex);
	}
	else
	{
		printf("close315 get g_485Mutex Lock fail!\n");
	}
}


/****************************************************************************
 * close315_action ����ң�ػ��ѣ�������
 * liushuhe
 * 2017.10.09
 ****************************************************************************/
void close315_action(int signo,siginfo_t *siginfo, void *arg)
{
	static int cnt; 
	if (signo == SIGUSR1)
	{
		printf("%2d SIGUSR1 received\n",cnt++);
		close_signal_flag = 1;
	}
}


/****************************************************************************
 * closelock_315
 * liushuhe
 * 2017.10.09
 ****************************************************************************/
int closelock_315(int argc, char *argv[])
{
	enum gpio_pintype_e pintype;
	struct sigaction act;
	struct sigaction oldact;
	int ret;
	int status;



  printf("close315: Terminating\n");

  return 0;
}






