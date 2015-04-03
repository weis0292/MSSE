/*
 * motor.h
 *
 * Created: 3/16/2015 8:40:51 PM
 *  Author: WeispfeM
 */ 

#include <math.h>
#include <stdlib.h>
#include <stdbool.h>

#include <pololu/orangutan.h>

#ifndef MOTOR_H_
#define MOTOR_H_

#define PWM IO_D7
#define DIR IO_C7
#define FORWARD LOW
#define REVERSE HIGH

void motor_init()
{
	TCCR2A = 0x03;
	TCCR2B = 0x02;

	set_digital_output(DIR, FORWARD);
	set_digital_output(PWM, LOW);
}

void set_motor(int speed)
{
	bool reverse = speed < 0;
	speed = fmin(abs(speed), 0xFF);
	OCR2A = speed;

	if (speed == 0)
	{
		TCCR2A &= ~(1 << COM2A1);
	}
	else
	{
		TCCR2A |= 1 << COM2A1;
		set_digital_output(DIR, reverse ? REVERSE : FORWARD);
	}
}

#endif /* MOTOR_H_ */