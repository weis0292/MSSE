/*
 * serial.h
 *
 * Created: 3/26/2015 8:10:04 PM
 *  Author: WeispfeM
 */ 

#ifndef SERIAL_H_
#define SERIAL_H_

#include <stdbool.h>

#include <pololu/orangutan.h>

#define BAUD_RATE 9600

char receive_buffer[255];
char send_buffer[255];
unsigned char receive_buffer_position = 0;

bool (*byte_received)(char byte);
void process_received_byte(char byte);

void configure_serial_port(bool (*on_byte_received)(char byte))
{
	byte_received = on_byte_received;

	serial_set_baud_rate(USB_COMM, BAUD_RATE);
	serial_receive_ring(USB_COMM, receive_buffer, sizeof(receive_buffer));
}

void check_for_new_bytes_received()
{
	while(serial_get_received_bytes(USB_COMM) != receive_buffer_position)
	{
		process_received_byte(receive_buffer[receive_buffer_position]);

		if (receive_buffer_position == sizeof(receive_buffer) - 1)
		{
			receive_buffer_position = 0;
		}
		else
		{
			receive_buffer_position++;
		}
	}
}

void wait_for_sending_to_finish()
{
	unsigned long timeout = 100;
	while (!serial_send_buffer_empty(USB_COMM) && (timeout-- > 0))
	serial_check();
}

void process_received_byte(char byte)
{
	if ((*byte_received)(byte))
	{
		wait_for_sending_to_finish();
		send_buffer[0] = byte;
		serial_send(USB_COMM, send_buffer, 1);
	}
}

void print_to_serial(char* output)
{
	//memset(send_buffer, 0, sizeof(send_buffer));
	//sprintf(send_buffer, output);
	//wait_for_sending_to_finish();
	//serial_send(USB_COMM, send_buffer, sizeof(send_buffer));
	//wait_for_sending_to_finish();
}

#endif /* SERIAL_H_ */