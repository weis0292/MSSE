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

#define NUNCHUCK_ADDRESS 0x52

bool is_initialized = false;

void nunchuck_initialize()
{
	twi_start((NUNCHUCK_ADDRESS << 1) | TW_WRITE);
	twi_write(0x40);
	twi_write(0x00);
	twi_stop();

	is_initialized = true;
}

void nunchuck_check_is_initialized()
{
	if (!is_initialized)
	{
		nunchuck_initialize();
	}
}

void nunchuck_conversion_command()
{
	nunchuck_check_is_initialized();

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

	// This delay was added due to testing.
	// No delay causes a read error on the
	// next communication with the nunchuck
	delay_ms(1);
}

unsigned char nunchuck_data[6];

unsigned char nunchuck_decode_byte(unsigned char byte)
{
	return (byte ^ 0x17) + 0x17;
}

void nunchuck_refresh_data()
{
	nunchuck_check_is_initialized();
	nunchuck_conversion_command();

	//unsigned char data = 0x00;

	//delay_ms(100);

	uint8_t byte_count = 6;

	twi_start((NUNCHUCK_ADDRESS << 1) | TW_READ);
	for (uint8_t i = 0; i < byte_count; i++)
	{
		bool is_last_byte = (i == (byte_count - 1));
		unsigned char data = is_last_byte ? twi_read_with_nak() : twi_read_with_ack();
		nunchuck_data[i] = nunchuck_decode_byte(data);
		//if (i == 0)
		//{
		//data = (twi_read() ^ 0x17) + 0x17;
		////lcd_goto_xy(0, 1);
		////printf("%d", data);
		//}
		//else
		//{
		//twi_read();
		//}
	}
	//twi_read_nak();
	twi_stop();
	//delay_ms(100);

}

unsigned char nunchuck_get_joystick_x()
{
	return nunchuck_data[0];
	//nunchuck_check_is_initialized();
	//nunchuck_conversion_command();

	//unsigned char data = 0x00;

	//delay_ms(100);

	//twi_start((NUNCHUCK_ADDRESS << 1) | TW_READ);
	//for (int i = 0; i < 5; i++)
	//{
		//if (i == 0)
		//{
			//data = (twi_read() ^ 0x17) + 0x17;
			////lcd_goto_xy(0, 1);
			////printf("%d", data);
		//}
		//else
		//{
			//twi_read();
		//}
	//}
	////twi_read_nak();
	//twi_stop();
	//delay_ms(100);

	//return data;
}

unsigned char nunchuck_get_joystick_y()
{
	return nunchuck_data[1];
}

uint16_t nunchuck_get_accelerometer_x()
{
	return (nunchuck_data[2] << 2) | ((nunchuck_data[5] | 0x0C) >> 2);
}

uint16_t nunchuck_get_accelerometer_y()
{
	return (nunchuck_data[3] << 2) | ((nunchuck_data[5] | 0x30) >> 4);
}

uint16_t nunchuck_get_accelerometer_z()
{
	return (nunchuck_data[4] << 2) | ((nunchuck_data[5] | 0xC0) >> 6);
}

bool nunchuck_get_button_c()
{
	return (nunchuck_data[5] & 0x02) == 0;
}

bool nunchuck_get_button_z()
{
	return (nunchuck_data[5] & 0x01) == 0;
}
