/****************************************************************************
 * examples/NB-main.c
 *
 *   Copyright (C) 2016 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

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
#include "NB_Iot.h"
#include "EC20.h"
#include "485.h"
#include "DeviceInfo.h"

/****************************************************************************
 * Private Data
 ****************************************************************************/

static bool  g_NB_Iot_started;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * NB_Iot_main
 ****************************************************************************/

#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int NB_IOT_main(int argc, FAR char *argv[])
#endif
{
	int ret;
	DeviceInfoInit();

	printf("NB_Iot_main: Starting the NB_Iot_main\n");
	if (g_NB_Iot_started)
	{
		printf("NB_Iot_main: NB_Iot_main already running\n");
		return EXIT_SUCCESS;
	}
	g_NB_Iot_started = 1;
#if 0	
	ret = task_create("EC20", CONFIG_EXAMPLES_NB_IOT_PRIORITY,
	                CONFIG_EXAMPLES_NB_IOT_STACKSIZE, EC20,
	                NULL);
	if (ret < 0)
	{
		int errcode = errno;
		printf("NB_Iot_main: ERROR: Failed to start NB_Iot: %d\n",
		     errcode);
		return EXIT_FAILURE;
	}
#endif
#if 1
	ret = task_create("NB_Iot", CONFIG_EXAMPLES_NB_IOT_PRIORITY,
	                CONFIG_EXAMPLES_NB_IOT_STACKSIZE, NB_Iot,
	                NULL);
	if (ret < 0)
	{
		int errcode = errno;
		printf("NB_Iot_main: ERROR: Failed to start NB_Iot: %d\n",
		     errcode);
		return EXIT_FAILURE;
	}
	printf("NB_Iot_main: NB_Iot started\n");
#endif
#if 1
	ret = task_create("_485", CONFIG_EXAMPLES_NB_IOT_PRIORITY,
	                CONFIG_EXAMPLES_NB_IOT_STACKSIZE, _485,
	                NULL);
	if (ret < 0)
	{
		int errcode = errno;
		printf("NB_Iot_main: ERROR: Failed to start NB_Iot: %d\n",
		     errcode);
		return EXIT_FAILURE;
	}
#endif
	printf("NB_Iot_main: NB_Iot started\n");
	
	while(1)
	{
		sleep(1);
	}
	return EXIT_SUCCESS;
}
