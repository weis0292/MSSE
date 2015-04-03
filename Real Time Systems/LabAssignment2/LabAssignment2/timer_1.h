/*
 * timer_1.h
 *
 * Created: 3/9/2015 8:57:28 PM
 *  Author: WeispfeM
 */ 

#ifndef TIMER_1_H_
#define TIMER_1_H_

#include <stdbool.h>

#include <pololu/orangutan.h>

void timer_1_init()
{
	TCCR1A = 0x80;
	TCCR1B = 0x1B;
	TCCR1C = 0x00;
	ICR1 = 0xF424;
	TIMSK1 = 0x02;
	sei();
}

volatile bool g_release_print_task = false;

ISR (TIMER1_COMPA_vect)
{
	g_release_print_task = true;
}

#endif /* TIMER_1_H_ */