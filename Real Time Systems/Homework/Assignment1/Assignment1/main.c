/* Assignment1 - an application for the Pololu Orangutan SVP
 *
 * This application uses the Pololu AVR C/C++ Library.  For help, see:
 * -User's guide: http://www.pololu.com/docs/0J20
 * -Command reference: http://www.pololu.com/docs/0J18
 *
 * Created: 1/26/2015 8:25:18 PM
 *  Author: Mike
 */

#include <pololu/orangutan.h>

int main()
{
	clear();

	while(1)
	{
		unsigned char button = wait_for_button_press(ANY_BUTTON);
		
		if(button == TOP_BUTTON)
		{
			print("Top Button");
			green_led(HIGH);
		}
		else if(button == MIDDLE_BUTTON)
		{
			print("Middle Button");
			green_led(HIGH);
			red_led(HIGH);
		}
		else if(button == BOTTOM_BUTTON)
		{
			print("Bottom Button");
			red_led(HIGH);
		}
		
		wait_for_button_release(button);

		green_led(LOW);
		red_led(LOW);

		clear();
	}
}
