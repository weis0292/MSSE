/*
 * nintendo_nunchuk.c
 *
 * Created: 4/3/2015 10:28:45 AM
 * Author: WeispfeM
 */

#include <stdbool.h>

#include <pololu/orangutan.h>
#include <compat/twi.h>

#include "../headers/nintendo_nunchuk.h"
#include "../headers/two_wire_interface.h"
//#include "../headers/i2cmaster.h"

#define NUNCHUCK_ADDRESS 0x52

bool is_initialized = false;

void initialize()
{
	//twi_init();
	//delay_ms(100);
	twi_start((NUNCHUCK_ADDRESS << 1) | TW_WRITE);
	twi_write(0x40);
	twi_write(0x00);
	twi_stop();
	delay_ms(100);

	is_initialized = true;
}

void check_is_initialized()
{
	if (!is_initialized)
	{
		initialize();
	}
}

unsigned char nunchuck_get_joystick_x()
{
	check_is_initialized();

	unsigned char data = 0x00;

	delay_ms(100);
	twi_start((NUNCHUCK_ADDRESS << 1) | TW_WRITE);
	//twi_start(0xA4);
	//twi_write(0x40);
	//delay_ms(100);
	//if (i2c_write(0x00) > 0)
	//{
		//lcd_goto_xy(0, 1);
		//printf("Error Write");
	//}
	//delay_ms(100);
	twi_write(0x00);
	twi_stop();
	delay_ms(100);

	twi_start((NUNCHUCK_ADDRESS << 1) | TW_READ);
	for (int i = 0; i < 6; i++)
	{
		if (i == 0)
		{
			data = (twi_read() ^ 0x17) | 0x17;
			//lcd_goto_xy(0, 1);
			//printf("%d", data);
		}
		else
		{
			twi_read();
		}
	}
	twi_stop();
	delay_ms(100);

	return data;
}