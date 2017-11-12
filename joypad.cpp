#include <stdio.h>
#include "include/joypad.h"
#include "main.h"

int joy_fd;
int axis[29];       //29 �향
char button[17];    //17개 버튼
int num_of_axis = 0;
int num_of_buttons = 0;
char name_of_joystick[80];
int led_ctl = 0;

struct js_event js;

typedef enum _LED {
    led_ctl_1 = 1,
    led_ctl_2 = 2,
    led_ctl_3 = 3,
    led_ctl_4 = 4
} _LED;

extern void control_accel( int throtle );

int joypad_init(void)
{
    info.direct = 0;

    MSG("joypad_init()");
    if( ( joy_fd = open( JOY_DEV , O_RDONLY)) == -1 ){
        printf( "Couldn't open joystick\n" );
        MSG("joypad_init fail!");
        return -1;
    }

    ioctl( joy_fd, JSIOCGAXES, &num_of_axis );
    ioctl( joy_fd, JSIOCGBUTTONS, &num_of_buttons );
    ioctl( joy_fd, JSIOCGNAME(80), &name_of_joystick );

    //axis = (int *) malloc( num_of_axis, sizeof( int ) );
    //button = (char *) malloc( num_of_buttons, sizeof( char ) );

    printf("Joystick detected: %s\n\t%d axis\n\t%d buttons\n\n", 
                        name_of_joystick, num_of_axis, num_of_buttons );
    fcntl( joy_fd, F_SETFL, O_NONBLOCK );   /* use non-blocking mode */

    return 0;
}

void joypad_task(void)
{
    int i;

    while(1){
        usleep(3000);
        /* read the joystick state */
        read(joy_fd, &js, sizeof(struct js_event));

        /* see what to do with the event */
        switch (js.type & ~JS_EVENT_INIT){
            case JS_EVENT_AXIS:
            axis[ js.number ] = js.value;
            break;
            case JS_EVENT_BUTTON:
            button[ js.number ] = js.value;
            break;
        }

        /* print the results axis[1] 세로 */
        printf( "X: %6d  Y: %6d  ", axis[0], axis[1] );
        control_accel(axis[1]);

        if( num_of_axis > 2 )
            printf("Z: %6d  ", axis[2] );

        if( num_of_axis > 3 )
            printf("R: %6d  ", axis[3] );

        if( num_of_axis > 4 )
            printf("Z: %6d  ", axis[4] );

        if( num_of_axis > 5 )
            printf("R: %6d  ", axis[5] );

        for( i=0 ; i<num_of_buttons ; ++i ){
            if( button[i] ){
                led_ctl =  led_ctl | (1 << i);
            } else {
                led_ctl =  led_ctl & ~(1 << i);
            }
            //printf("B%d: %d  ", x, button[i] );
        }

        printf("  %d  ",led_ctl );
        //set_led_ctrl( led_ctl );

        printf("  \r");
        fflush(stdout);
    }
    close( joy_fd );        /* too bad we never get here */
}