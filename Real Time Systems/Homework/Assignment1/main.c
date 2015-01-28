/* Assignment1 - an application for the Pololu Orangutan SVP
 *
 * This application uses the Pololu AVR C/C++ Library.  For help, see:
 * -User's guide: http://www.pololu.com/docs/0J20
 * -Command reference: http://www.pololu.com/docs/0J18
 *
 * Created: 1/26/2015 9:04:18 PM
 *  Author: Mike
 * This program will read the input of the three buttons.  If any of the buttons
 * are pressed the program will blink the appropriate LED for as long as the button
 * is being held.
 */

#include <pololu/orangutan.h>

// The time an LED will stay at a given state when the button is pressed
const long blink_ms = 250;

void ToggleLEDIfButtonPressed(unsigned char pressed_state, unsigned long *ticks, void (*led_method)(unsigned char));

int main()
{
	clear();
	
	// The amount of time the button has been held since the last LED toggle
	unsigned long ticks_top = 0;
	unsigned long ticks_bottom = 0;

	while(1)
	{
		unsigned char pressed_state = button_is_pressed(ANY_BUTTON);
		// The middle button will count as both the top and the bottom button (fun!)
		unsigned char pressed_state_top = pressed_state & (TOP_BUTTON | MIDDLE_BUTTON);
		unsigned char pressed_state_bottom = pressed_state & (BOTTOM_BUTTON | MIDDLE_BUTTON);

		// Deal with the top button and the green LED
		ToggleLEDIfButtonPressed(pressed_state_top, &ticks_top, green_led);

		// Deal with the bottom button and the red LED
		ToggleLEDIfButtonPressed(pressed_state_bottom, &ticks_bottom, red_led);
	}
}

// Here we can put common code that is done for both LEDs and just pass in the led function
void ToggleLEDIfButtonPressed(unsigned char pressed_state, unsigned long *ticks, void (*led_method)(unsigned char))
{
	// Check if button is pressed
	if(pressed_state)
	{
		// See how long the button has been pressed
		unsigned long elapsed_time_ms = (ticks_to_microseconds(get_ticks() - *ticks)) / 1000;
		if((elapsed_time_ms >= blink_ms) || (*ticks == 0))
		{
			// Time has elapsed, toggle LED and set new ticks.
			(*led_method)(TOGGLE);
			*ticks = get_ticks();
		}
	}
	else
	{
		// No button is pressed, reset LED and ticks
		(*led_method)(LOW);
		*ticks = 0;
	}
}
