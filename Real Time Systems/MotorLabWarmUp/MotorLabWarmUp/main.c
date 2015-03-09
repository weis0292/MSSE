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
bool g_last_m1a_val = 0;
bool g_last_m1b_val = 0;
long g_counts_m1 = 0;

// Motor speed variables
int g_desired_speed = 0;
int g_current_speed = 0;
int g_motor_speed_change = 0x08;
int g_max_motor_speed = 0xFF;
int g_min_motor_speed = -0xFF;

// Task release variable
bool g_release_print_task = false;
bool g_release_motor_change_task = false;

void lcd_init();
void print_motor_information();
void move_motor_to_desired();

// This is the interrupt that handles the pin
// changes on the A and B channels of the motor
// encoder.
void pc_interrupt_init();

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
	pc_interrupt_init();
	timer1_init();
	timer3_init();
	sei();

	while (true)
	{
		unsigned char button = get_single_debounced_button_press(ANY_BUTTON);
		if ((button & TOP_BUTTON) > 0)
		{
			// Let's stop by zero on the way by
			if ((g_desired_speed < 0) && (g_desired_speed + g_motor_speed_change > 0))
			{
				g_desired_speed = 0;
			}
			else
			{
				g_desired_speed = fmin(g_max_motor_speed, g_desired_speed + g_motor_speed_change);
			}
		}
		if ((button & MIDDLE_BUTTON) > 0)
		{
			g_desired_speed = -g_desired_speed;
		}
		if ((button & BOTTOM_BUTTON) > 0)
		{
			// Let's stop by zero on the way by
			if ((g_desired_speed > 0) && (g_desired_speed - g_motor_speed_change < 0))
			{
				g_desired_speed = 0;
			}
			else
			{
				g_desired_speed = fmax(g_min_motor_speed, g_desired_speed - g_motor_speed_change);
			}
		}

		if (g_release_motor_change_task)
		{
			g_release_motor_change_task = false;
			move_motor_to_desired();
		}

		if (g_release_print_task)
		{
			g_release_print_task = false;
			print_motor_information();
		}
	}
}

void lcd_init()
{
	clear();
	lcd_init_printf();
}

void pc_interrupt_init()
{
	// Enable pin change interrupt 3
	PCICR = 0x08;
	// Set encoder pins D2 and D3 to
	// trigger the pin change interrupt
	PCMSK3 = 0x0C;
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

void print_motor_information()
{
	// Print the current motor speed
	lcd_goto_xy(0, 0);
	printf("M1 Speed: %4i", g_current_speed);

	// Print the encoder count information
	lcd_goto_xy(0, 1);
	printf("Enc: %9ld", g_counts_m1);
}

void move_motor_to_desired()
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

ISR (PCINT3_vect)
{
	// Get the current state of the encoder
	bool m1a_val = (PIND & (1 << IO_D3)) > 0;
	bool m1b_val = (PIND & (1 << IO_D2)) > 0;

	// Determine which way the encoder is turning
	// using the previous state
	bool plus_m1 = m1a_val ^ g_last_m1b_val;
	bool minus_m1 = m1b_val ^ g_last_m1a_val;

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
	g_release_print_task = true;
}

ISR (TIMER3_COMPA_vect)
{
	g_release_motor_change_task = true;
}