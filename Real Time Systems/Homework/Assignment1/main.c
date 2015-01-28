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
const unsigned long _blinkMS = 250;

void ToggleLEDIfButtonPressed(unsigned char pressedState, unsigned long *ticks, void (*ledMethod)(unsigned char));

int main()
{
	clear();
	
	// The amount of time the button has been held since the last LED toggle
	unsigned long ticksTop = 0;
	unsigned long ticksBottom = 0;

	while(1)
	{
		unsigned char pressedState = button_is_pressed(ANY_BUTTON);
		// The middle button will count as both the top and the bottom button (fun!)
		unsigned char pressedStateTop = pressedState & (TOP_BUTTON | MIDDLE_BUTTON);
		unsigned char pressedStateBottom = pressedState & (BOTTOM_BUTTON | MIDDLE_BUTTON);

		// Deal with the top button and the green LED
		ToggleLEDIfButtonPressed(pressedStateTop, &ticksTop, green_led);

		// Deal with the bottom button and the red LED
		ToggleLEDIfButtonPressed(pressedStateBottom, &ticksBottom, red_led);
	}
}

// Here we can put common code that is done for both LEDs and just pass in the led function
void ToggleLEDIfButtonPressed(unsigned char pressedState, unsigned long *ticks, void (*ledMethod)(unsigned char))
{
	// Check if button is pressed
	if(pressedState)
	{
		// See how long the button has been pressed
		unsigned long elapsedTimeMS = (ticks_to_microseconds(get_ticks() - *ticks)) / 1000;
		if((elapsedTimeMS >= _blinkMS) || (*ticks == 0))
		{
			// Time has elapsed, toggle LED and set new ticks.
			(*ledMethod)(TOGGLE);
			*ticks = get_ticks();
		}
	}
	else
	{
		// No button is pressed, reset LED and ticks
		(*ledMethod)(LOW);
		*ticks = 0;
	}
}
