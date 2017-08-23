#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/joystick.h>

#ifndef JOYPAD_H
#define JOYPAD_H

#define JOY_DEV "/dev/input/js0"

extern int joypad_init(void);
extern void joypad_task(void);
#endif