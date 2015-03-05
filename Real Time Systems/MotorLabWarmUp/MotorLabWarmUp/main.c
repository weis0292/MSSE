/* MotorLabWarmUp - an application for the Pololu Orangutan SVP
 *
 * This application uses the Pololu AVR C/C++ Library.  For help, see:
 * -User's guide: http://www.pololu.com/docs/0J20
 * -Command reference: http://www.pololu.com/docs/0J18
 *
 * Created: 3/3/2015 9:56:20 PM
 *  Author: WeispfeM
 */

#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <pololu/orangutan.h>

// Encoder variables
unsigned char g_last_m1a_val = 0;
unsigned char g_last_m1b_val = 0;
long g_counts_m1 = 0;

// Motor speed variables
int g_desired_speed = 0;
int g_current_speed = 0;
int g_motor_speed_change = 0x05;
int g_max_motor_speed = 0xFF;
int g_min_motor_speed = -0xFF;

void lcd_init();
// Timer 0 is responsible for keeping track of the
// encoder counts for the motor(s).  The timer is
// set to fire an interrupt every 100 microseconds.
void timer0_init();
// Timer 1 is responsible for printing values out
// to the screen.  It was determined that the screen
// couldn't handle being written to as often as we
// have updated values, so the timer is currently
// set to interrupt every 200 milliseconds.
void timer1_init();
// Timer 3 is responsible for changing the motor
// speed to the desired speed.  This functionality
// was originally in timer 1, but the 200 milliseconds
// that timer was running at was deemed too slow.
// As such, the timer is set to interrupt every
// 20 milliseconds.  This gives a good rate of
// change, especially when reversing direction.
// The cyclic executive merely sets the desired
// motor speed, and Timer 3 makes sure the motor
// gets to that speed.
void timer3_init();

int main ()
{
	lcd_init();
	timer0_init();
	timer1_init();
	timer3_init();
	sei();

	while (true)
	{
		unsigned char button = get_single_debounced_button_press(ANY_BUTTON);
		if ((button & TOP_BUTTON) > 0)
		{
			g_desired_speed = fmin(g_max_motor_speed, g_desired_speed + g_motor_speed_change);
		}
		if ((button & MIDDLE_BUTTON) > 0)
		{
			g_desired_speed = -g_desired_speed;
		}
		if ((button & BOTTOM_BUTTON) > 0)
		{
			g_desired_speed = fmax(g_min_motor_speed, g_desired_speed - g_motor_speed_change);
		}
	}
}

void lcd_init()
{
	clear();
	lcd_init_printf();
}

void timer0_init()
{
	TCCR0A = 0x82;
	TCCR0B = 0x02;
	OCR0A = 0xFA;
	TIMSK0 = 0x02;
}

void timer1_init()
{
	TCCR1A = 0x80;
	TCCR1B = 0x1B;
	TCCR1C = 0x00;
	ICR1 = 0xF424;
	TIMSK1 = 0x02;
}

void timer3_init()
{
	TCCR3A = 0x80;
	TCCR3B = 0x1B;
	TCCR3C = 0x00;
	ICR3 = 0x186A;
	TIMSK3 = 0x02;
}

ISR (TIMER0_COMPA_vect)
{
	// Get the current state of the encoder
	unsigned char m1a_val = is_digital_input_high(IO_D3);
	unsigned char m1b_val = is_digital_input_high(IO_D2);

	// Determine which way the encoder is turning
	// using the previous state
	char plus_m1 = m1a_val ^ g_last_m1b_val;
	char minus_m1 = m1b_val ^ g_last_m1a_val;

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
	g_last_m1a_val = m1a_val;
	g_last_m1b_val = m1b_val;
}

ISR (TIMER1_COMPA_vect)
{
	// Print the current motor speed
	lcd_goto_xy(0, 0);
	printf("M1 Speed: %4i", g_current_speed);

	// Print the encoder count information
	lcd_goto_xy(0, 1);
	printf("Enc: %9ld", g_counts_m1);
}

ISR (TIMER3_COMPA_vect)
{
	if (g_current_speed != g_desired_speed)
	{
		int desired_speed = 0;
		if (g_current_speed > g_desired_speed)
		{
			desired_speed = fmax(g_desired_speed, g_current_speed - g_motor_speed_change);
		}
		else
		{
			desired_speed = fmin(g_desired_speed, g_current_speed + g_motor_speed_change);
		}

		// Make sure the current speed is within the allowable limits and set the speed
		g_current_speed = fmin(g_max_motor_speed, fmax(g_min_motor_speed, desired_speed));
		set_m1_speed(g_current_speed);
	}
}