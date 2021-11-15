#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>   //exit

#include <pthread.h>
#include "curses.h"
#include <linux/uinput.h>

#include "include/joypad.h"
#include <wiringPi.h>

#include "main.h"
#include "control.h"
#include "ssd1306/ssd1306.h"

//Thread feature
#define NUM_THREADS 3

extern void motor_init(void);
extern void motor_clock_task(void);

_info info;         /* information */
SSD1306 myDisplay;  /* Define Local dispaly value */
     
/*********************************************************************/
/* ThreadProcs */
/*********************************************************************/
void *ThreadProcs(void *threadid)
{
    
    int i,k,m;
    int ch;
    int thread_id = (int)threadid;

    switch( thread_id ){
        case 0:
            motor_clock_task();
            break;
        case 1:
            joypad_task();
            break;
        case 2:
            //THIS THREAD WILL MAKE THE PROGRAM EXIT
            wmove(stdscr, 1, 0);
            addstr("Type \"q\" to quit.\n");
            nodelay(stdscr, TRUE);          //SETUP NON BLOCKING INPUT
            while(1) {
                if ((ch = getch()) == ERR){
                    usleep(16666);          //USER HASN'T RESPONDED
                } else if( ch == 'q'){
                    endwin();
                    exit(0);                //QUIT ALL THREADS
                }
#if 0
            while(10)
            {
                m = 0;
                for(i=0; i<64; i++) {
                    for(k=0; k<128; k++) {
                        SSD1306_buffer[m] = 0;
                        m++;
                    }
                };
                display();

                sleep(5);
                m = 0;
                for(i=0; i<64; i++) {
                    for(k=0; k<128; k++) {
                        SSD1306_buffer[m] = 0x0f0;
                        m++;
                    }
                }
                display();
                sleep(5);
            }
#endif
            }
            break;
        default:
            break;
    }
}

int main(int argc, char * argv [] )
{
    pthread_t threads[NUM_THREADS];
    int rc, t;

    wiringPiSetupGpio();

    SSD1306_init();
    myDisplay.setDisplayMode(SSD1306::Mode::SCROLL);
    myDisplay.setWordWrap(TRUE);
    myDisplay.textDisplay("THIS IS THE FIRST LINE.");

    motor_init();
    joypad_init();

    for(t = 0; t < NUM_THREADS; t++){
        rc = pthread_create(&threads[t], NULL, ThreadProcs, (void *)t);
        if (rc){
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
            pthread_exit(NULL);
        }
    }
     
    for(t = 0; t < NUM_THREADS; t++){
        //WAIT FOR THREADS TO EXIT OR IT WILL RACE TO HERE.
        pthread_join(threads[t], NULL);
    }
    endwin();
    return 0;
}
