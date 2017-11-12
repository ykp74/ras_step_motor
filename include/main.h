#include "../ssd1306/ssd1306.h"

#ifndef _MAIN_H
#define _MAIN_H
/*
    main header file by KSND
*/

typedef enum dir {
    EFORWARD = 0,
    EBACK,
    ERIGHT,
    ELEFT,
    MAX
}dir;

typedef struct _info{
    int speed;
    char direct;
    char status = 0;
}_info;

extern _info info;         /* information */
extern SSD1306 myDisplay;  /* Define Local dispaly value */

#define MIN_SPEED 327670
#define MAX_SPEED 1

#define STATUS_FORWARD  0x1
#define STATUS_BACKWARD 0x2
#define STATUS_RIGHT    0x4
#define STATUS_LEFT     0x8

#define MSG(M)  myDisplay.textDisplay(M)

#endif