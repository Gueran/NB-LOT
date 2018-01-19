#ifndef __GPIO_H_H__
#define __GPIO_H_H__
int GPIO_read(int *fd);

int GPIO_write(int *fd,  bool outvalue);
int GPIO_INTERRUPT(int *fd,int signo,int sec);
#endif