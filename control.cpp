#include <stdio.h>
#include <stdint.h>
#include "main.h"
#include "control.h"
#include "ssd1306/ssd1306.h"

char current_status = 0;
char buffer[20];

extern SSD1306 myDisplay;

void control_display(void)
{
    if( current_status != info.status){
        sprintf(buffer, "DIRECTION : %d", info.status );
        myDisplay.textDisplay(buffer);
        current_status =  info.status;
    }
}

void control_accel( int throtle )
{
    if( throtle == 0 ){
        info.status &= ~STATUS_BACKWARD;
        info.status &= ~STATUS_FORWARD;
        info.speed = MIN_SPEED;
    } else if( throtle > 0){
        info.status |= STATUS_BACKWARD;
        info.status &= ~STATUS_FORWARD;
        if( throtle == 32767){
            info.speed = MAX_SPEED;
        } else {
            info.speed = (32767 - throtle) * 10; 
        }
    } else if( throtle < 0 ) {
        info.status |= STATUS_FORWARD;
        info.status &= ~STATUS_BACKWARD;
        if( throtle == -32767){
            info.speed = MAX_SPEED;
        } else {
            info.speed = (32767 + throtle) * 10; 
        }
    }
}