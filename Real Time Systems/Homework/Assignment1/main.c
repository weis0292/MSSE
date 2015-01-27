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

void ClearAll()
{
	green_led(LOW);
	red_led(LOW);
	clear();
}

int main()
{
	ClearAll();

	while(1)
	{
		unsigned char button = wait_for_button_press(ANY_BUTTON);
		if(button == TOP_BUTTON)
		{
			green_led(HIGH);
			print("Top Button");
		}
		else if(button == MIDDLE_BUTTON)
		{
			green_led(HIGH);
			red_led(HIGH);
			print("Middle Button");
		}
		else if(button == BOTTOM_BUTTON)
		{
			red_led(HIGH);
			print("Bottom Button");
		}
		
		wait_for_button_release(button);
		ClearAll();
	}
}