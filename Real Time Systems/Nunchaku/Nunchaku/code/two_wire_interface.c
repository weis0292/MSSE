/*
 * two_wire_interface.c
 *
 * Created: 4/3/2015 10:28:20 AM
 * Author: Finga Gunz
 */

#ifndef F_CPU
#define F_CPU 20000000UL
#endif

#ifndef SCL_CLOCK
#define SCL_CLOCK 100000UL
#endif

#include <stdbool.h>
#include <stdio.h>
#include <pololu/orangutan.h>
#include <compat/twi.h>

#include "../headers/two_wire_interface.h"

bool is_twi_initialized = false;

void twi_initialize()
{
	TWSR = 0;
	TWBR = ((F_CPU / SCL_CLOCK) - 16) / 2;

	//set_digital_output(IO_C0, HIGH);
	//set_digital_output(IO_C1, HIGH);

	is_twi_initialized = true;
}

void twi_verify_is_initialized()
{
	if (!is_twi_initialized)
	{
		twi_initialize();
	}
}

void twi_wait_for_twint_flag_set()
{
	while (!(TWCR & (1 << TWINT)));
}

void twi_start(unsigned char address)
{
	twi_verify_is_initialized();

	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);

	twi_wait_for_twint_flag_set();

	if (((TW_STATUS & 0xF8) != TW_START) && ((TW_STATUS & 0xF8) != TW_REP_START))
	{
		lcd_goto_xy(0, 1);
		printf("ERROR start");
		return;
	}

	TWDR = address;
	TWCR = (1 << TWINT) | (1 << TWEN);

	twi_wait_for_twint_flag_set();

	if (((TW_STATUS & 0xF8) != TW_MT_SLA_ACK) && ((TW_STATUS & 0xF8) != TW_MR_SLA_ACK))
	{
		lcd_goto_xy(0, 1);
		printf("ERROR address");
	}
}

void twi_stop()
{
	twi_verify_is_initialized();

	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
	while (TWCR & (1 << TWSTO)) { }

	//lcd_goto_xy(0, 1);
	//printf("stop success");
}

void twi_write(unsigned char data)
{
	TWDR = data;
	TWCR = (1 << TWINT) | (1 << TWEN);

	twi_wait_for_twint_flag_set();

	if ((TW_STATUS & 0xF8) != TW_MT_DATA_ACK)
	{
		lcd_goto_xy(0, 1);
		printf("ERROR Write");
	}
	lcd_goto_xy(12, 1);
	printf("%d", TW_STATUS & 0xF8);
}

unsigned char twi_read_with_ack()
{
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);

	twi_wait_for_twint_flag_set();

	return TWDR;
}

unsigned char twi_read_with_nak()
{
	TWCR = (1 << TWINT) | (1 << TWEN);

	twi_wait_for_twint_flag_set();

	return TWDR;
}