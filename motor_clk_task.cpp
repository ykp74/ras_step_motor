#include <stdio.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include "main.h"
#include "control.h"

#define MOTOR_CLK_PIN   16

void motor_init(void)
{
    info.speed = MIN_SPEED;
    pinMode(MOTOR_CLK_PIN, OUTPUT);
    digitalWrite(MOTOR_CLK_PIN, HIGH);
    myDisplay.textDisplay("motor_init()");
}

void motor_clock_task(void)
{
    myDisplay.textDisplay("motor_clock_task:start");
    while(1){
        control_display();
        if( 1 /*info.speed < MIN_SPEED*/){
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