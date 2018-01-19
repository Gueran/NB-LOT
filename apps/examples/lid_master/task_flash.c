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

#include <nuttx/mtd/mtd.h>
#include <nuttx/drivers/drivers.h>
#include <nuttx/fs/ioctl.h>


#include "task_monitor.h"
#include "task_flash.h"

/****************************************************************************
 * Private Data
 ****************************************************************************/

struct alarm_value  alarmdata;

/****************************************************************************
 * mid
 * liushuhe
 * 2017.10.23
 ****************************************************************************/


/****************************************************************************
 * slave_flash
 * liushuhe
 * 2017.10.23
 ****************************************************************************/

int master_flash(int argc, char *argv[])
{
    char   *pcTempBuf[255];
    char   *pChar = ";";
	int		cCharNum = 0;

	FAR uint32_t *buffer;
	ssize_t nbytes;
	int fd;

	
  return 0;
}



