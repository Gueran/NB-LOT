
#include <nuttx/config.h>

#include <sys/ioctl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

#include <nuttx/ioexpander/gpio.h>

#include "GPIO.h"

int GPIO_read(int *fd)
{
	int ret;
	bool invalue;
	ret = ioctl(*fd, GPIOC_READ, (unsigned long)((uintptr_t)&invalue));
	if (ret < 0)
	{
		int errcode = errno;
		fprintf(stderr, "ERROR: Failed to read value from %d: %d\n", *fd, errcode);
		close(*fd);
		*fd = -1;
		return EXIT_FAILURE;
	}
	return invalue;
}

int GPIO_write(int *fd,  bool outvalue)
{
	int ret;
	ret = ioctl(*fd, GPIOC_WRITE, (unsigned long)outvalue);
	if (ret < 0)
	{
		int errcode = errno;
		fprintf(stderr, "ERROR: Failed to write value %u from %d: %d\n",
		     *fd, (unsigned int)outvalue, errcode);
		close(*fd);
		*fd = -1;
		return EXIT_FAILURE;
	}
	return 0;
}

int GPIO_INTERRUPT(int *fd,int signo,int sec)
{
	int ret;
	struct timespec ts;
	sigset_t set;
	
	/* Set up to receive signal */
	
	ret = ioctl(*fd, GPIOC_REGISTER, (unsigned long)signo);
	if (ret < 0)
	{
		int errcode = errno;
		fprintf(stderr, "ERROR: Failed to setup for signal from %d: %d\n",
			  *fd, errcode);
		close(*fd);
		return EXIT_FAILURE;
	}
	
	/* Wait up to 5 seconds for the signal */
	
	(void)sigemptyset(&set);
	(void)sigaddset(&set, signo);
	
	ts.tv_sec	= sec;
	ts.tv_nsec  = 0;
	ret = sigtimedwait(&set, NULL, &ts);
    (void)ioctl(fd, GPIOC_UNREGISTER, 1);
	return ret;
}

