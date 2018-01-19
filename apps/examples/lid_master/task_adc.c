#include <sys/ioctl.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sched.h>
#include <errno.h>

#include <nuttx/board.h>
#include <sys/boardctl.h>
#include <errno.h>
#include <nuttx/ioexpander/gpio.h>
#include <nuttx/analog/adc.h>
#include <nuttx/analog/ioctl.h>
#include <nuttx/leds/userled.h>
#include <nuttx/board.h>
#include <sys/boardctl.h>
#include <errno.h>
#include <nuttx/ioexpander/gpio.h>

#include "task_adc.h"
#include "task_monitor.h"
#include "task_485.h"

/****************************************************************************
 * Private Data
 ****************************************************************************/
 
pthread_mutex_t g_AdcMutex		= PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  g_AdcConVar	= PTHREAD_COND_INITIALIZER;

struct sensor_msg	SensorDate;
struct adc_msg		adcdata;
struct i2c_msg		i2cdata;

static bool  g_adc_started;



/****************************************************************************
 * slave_adc
 * liushuhe
 * 2017.09.26
 ****************************************************************************/
int master_adc(int argc, char *argv[])
{
	g_adc_started = true;
	
	SensorDate.adcmsg	= &adcdata;
	SensorDate.i2cmsg	= &i2cdata;
	


  printf("adc: Terminating\n");

  return 0;
}



