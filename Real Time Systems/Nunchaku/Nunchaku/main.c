/*
 * main.c
 *
 * Created: 4/3/2015 10:21:24 AM
 * Author: Finga Gunz
 */

#include <stdbool.h>
#include <stdio.h>

#include <pololu/orangutan.h>

#include "headers/nintendo_nunchuk.h"

int main()
{
	clear();
	lcd_init_printf();


	while(true)
	{
	unsigned char joystick_x = nunchuck_get_joystick_x();

	lcd_goto_xy(0, 0);
	printf("%d", joystick_x);
	delay_ms(100);
	}
}
