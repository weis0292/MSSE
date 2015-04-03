/*
 * pc_interrupt.h
 *
 * Created: 3/9/2015 8:41:06 PM
 *  Author: WeispfeM
 */ 

#ifndef PC_INTERRUPT_H_
#define PC_INTERRUPT_H_

#include <stdbool.h>

#include <pololu/orangutan.h>

void pc_interrupt_init()
{
	// Enable Pin Change Interrupt 3
	PCICR = 0x08;

	// Set I/O Pins D2 and D3 to
	// trigger Pin Change Interrupt 3
	PCMSK3 = 0x0C;
}

bool g_last_m1a = false;
bool g_last_m1b = false;
long g_counts_m1 = 0;

unsigned long microseconds = 0;
unsigned long last_ticks = 0;

ISR (PCINT3_vect)
{
	// Use the code below when determining our fastest
	// encoder rate, and our slowest encoder rate.
	// Simply put last_ticks into the print method in
	// main.c, and it will output on the LCD screen.
	unsigned long ticks = get_ticks();
	microseconds = ticks_to_microseconds(ticks - last_ticks);
	last_ticks = ticks;

	// Get the current state of the encoder
	bool m1a = (PIND & (1 << IO_D3)) > 0;
	bool m1b = (PIND & (1 << IO_D2)) > 0;

	// Determine which way the encoder is turning
	// using the previous state
	bool plus_m1 = m1a ^ g_last_m1b;
	bool minus_m1 = m1b ^ g_last_m1a;

	// Increment the encoder counts if the motor
	// is turning in the positive direction
	if(plus_m1)
	{
		g_counts_m1++;
	}
	// Decrement the encoder counts if the motor
	// is turning in the negative direction
	if(minus_m1)
	{
		g_counts_m1--;
	}

	// Save this state for next time
	g_last_m1a = m1a;
	g_last_m1b = m1b;
}

#endif /* PC_INTERRUPT_H_ */