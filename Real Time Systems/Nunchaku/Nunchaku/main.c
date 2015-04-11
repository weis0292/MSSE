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

void print_nunchuck_info();

int main()
{
	clear();
	lcd_init_printf();

	while(true)
	{
		//unsigned char joystick_x = nunchuck_get_joystick_x();

		nunchuck_refresh_data();
		print_nunchuck_info();
		delay_ms(100);

		//clear();
		//lcd_goto_xy(0, 0);
		//printf("%3d", joystick_x);
		//delay_ms(100);
	}
	
	//while(true) { }
}

void print_nunchuck_info()
{
		clear();

		printf("%3d %3d %d %d",
			nunchuck_get_joystick_x(),
			nunchuck_get_joystick_y(),
			nunchuck_get_button_c(),
			nunchuck_get_button_z());

		lcd_goto_xy(0, 1);

		printf("%4d %4d %4d",
			nunchuck_get_accelerometer_x(),
			nunchuck_get_accelerometer_y(),
			nunchuck_get_accelerometer_z());
}