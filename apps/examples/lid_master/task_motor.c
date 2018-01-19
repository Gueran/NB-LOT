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
#include <nuttx/config.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <debug.h>
#include <nuttx/drivers/pwm.h>
#include <nuttx/ioexpander/gpio.h>

#include "task_485.h"
#include "task_motor.h"
#include "task_monitor.h"
#include "task_adc.h"

/****************************************************************************
 * Private Data
 ****************************************************************************/
int  fd_sensorA;
int  fd_sensorB;
int  fd_light;
int  fd_pwm;
int  buzzpwm_fd;

int  Locker;

struct	hall_sensor  Hall_Sensor;
struct 	pwm_state_s g_pwmstate;


int master_motor(int argc, char *argv[])
{
	
  printf("master_motor: Terminating\n");

	return 0;
}

