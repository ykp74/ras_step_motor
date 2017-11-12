#include <stdio.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include "main.h"

#define MOTOR_CLK_PIN   16

extern void control_display(void);

void motor_init(void)
{
    info.speed = MIN_SPEED;
    pinMode(MOTOR_CLK_PIN, OUTPUT);
    digitalWrite(MOTOR_CLK_PIN, HIGH);
    myDisplay.textDisplay("motor_init()");
}

void motor_clock_task(void)
{
    while(1){
        control_display();
        if( info.speed < MIN_SPEED){
            digitalWrite(MOTOR_CLK_PIN, HIGH);
            //myDisplay.textDisplay("MOTOR_CLK_PIN, HIGH");
            delayMicroseconds(info.speed);
            digitalWrite(MOTOR_CLK_PIN, LOW);
            //myDisplay.textDisplay("MOTOR_CLK_PIN, LOW");
            delayMicroseconds(info.speed);
        } 
        fflush(stdout);
    }
}