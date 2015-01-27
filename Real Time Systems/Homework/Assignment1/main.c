/* Assignment1 - an application for the Pololu Orangutan SVP
 *
 * This application uses the Pololu AVR C/C++ Library.  For help, see:
 * -User's guide: http://www.pololu.com/docs/0J20
 * -Command reference: http://www.pololu.com/docs/0J18
 *
 * Created: 1/26/2015 9:04:18 PM
 *  Author: Mike
 */

#include <pololu/orangutan.h>

int main()
{
	const long blink_ms = 250;
	unsigned long ticks_top = 0;
	unsigned long ticks_bottom = 0;

	while(1)
	{
		unsigned char pressed_state = button_is_pressed(ANY_BUTTON);
		// The middle button will count as both the top and the bottom button (fun!)
		unsigned char pressed_state_top = pressed_state & (TOP_BUTTON | MIDDLE_BUTTON);
		unsigned char pressed_state_bottom = pressed_state & (BOTTOM_BUTTON | MIDDLE_BUTTON);

		// Deal with the top button and the green LED
		if(pressed_state_top)
		{
			if(((ticks_to_microseconds(get_ticks() - ticks_top) / 1000) >= blink_ms) || (ticks_top == 0))
			{
				green_led(TOGGLE);
				ticks_top = get_ticks();
			}
		}
		else
		{
			green_led(LOW);
			ticks_top = 0;
		}

		// Deal with the bottom button and the red LED
		if(pressed_state_bottom)
		{
			if(((ticks_to_microseconds(get_ticks() - ticks_bottom) / 1000) >= blink_ms) || (ticks_bottom == 0))
			{
				red_led(TOGGLE);
				ticks_bottom = get_ticks();
			}
		}
		else
		{
			red_led(LOW);
			ticks_bottom = 0;
		}
	}
}