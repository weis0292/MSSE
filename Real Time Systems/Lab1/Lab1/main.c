/* Lab1 - an application for the Pololu Orangutan SVP
 *
 * This application uses the Pololu AVR C/C++ Library.  For help, see:
 * -User's guide: http://www.pololu.com/docs/0J20
 * -Command reference: http://www.pololu.com/docs/0J18
 *
 * Created: 2/13/2015 10:51:31 AM
 *  Author: WeispfeM
 */

#include <pololu/orangutan.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

// When testing with micro-seconds, this value got me to 10,001
// micro-seconds.  That value is going to have to be close enough.
#define FOR_COUNT_10MS 5700
volatile uint32_t __ii;
#define WAIT_10MS { for (__ii = 0; __ii < FOR_COUNT_10MS; __ii++); }

#define COMMAND_TYPE_NONE 'N'
#define COMMAND_TYPE_ZERO 'Z'
#define COMMAND_TYPE_TOGGLE 'T'
#define COMMAND_TYPE_PRINT 'P'

#define COMMAND_COLOR_NONE 'N'
#define COMMAND_COLOR_RED 'R'
#define COMMAND_COLOR_GREEN 'G'
#define COMMAND_COLOR_YELLOW 'Y'
#define COMMAND_COLOR_ALL 'A'

const unsigned long BAUD_RATE = 9600;
char receive_buffer[32];
char send_buffer[32];
unsigned char receive_buffer_position = 0;

void configure_serial_port();
void check_for_new_bytes_received();
void process_received_byte(char byte);
void wait_for_sending_to_finish();
void process_command(char type, char color, int time_ms);

char command_type = COMMAND_TYPE_NONE;
char command_color = COMMAND_COLOR_NONE;
char command_time_ms[5];
int command_time_ms_position = 0;

unsigned long count_green_toggle = 0;
unsigned long count_yellow_toggle = 0;
unsigned long count_red_toggle = 0;

unsigned long interrupts_per_toggle_16_bit = 2;
unsigned long interrupts_per_toggle_8_bit = 250;

float pwm_counts_per_ms = 19.53125;

void configure_8_bit_timer();
void configure_pwm();
void configure_16_bit_timer();

volatile bool release_red_led_task = false;

int main()
{
	// Clear the LCD display
	clear();
	
	configure_serial_port();

	configure_8_bit_timer();
	configure_pwm();
	configure_16_bit_timer();

	// Enable interrupts
	sei();

	while(true)
	{
		serial_check();
		check_for_new_bytes_received();
		
		//for(int i = 0; i < 50; i++)
		//{
			//WAIT_10MS;
		//}
		//set_digital_output(IO_A0, TOGGLE);
		//set_digital_output(IO_A2, TOGGLE);
		//set_digital_output(IO_D5, TOGGLE);

		if(release_red_led_task)
		{
			release_red_led_task = false;
			set_digital_output(IO_A2, TOGGLE);
			count_red_toggle++;
		}
	}

	// Use the below code to determine what value we should
	// use in the #define for-loop to get a 10 ms for loop
	//volatile long i;
	//unsigned long start_time = get_ms();
	//for (i = 0; i < 4000000; i++)
	//{
	//}
	//unsigned long end_time = get_ms();
	//unsigned long total_time = end_time - start_time;
	//print_unsigned_long(total_time);
	//while(true)
	//{
		//unsigned long start_time = get_ticks();
		//WAIT_10MS;
		//unsigned long end_time = get_ticks();

		//lcd_goto_xy(0, 0);
		//print_unsigned_long(ticks_to_microseconds(end_time - start_time));
	//}
}

void configure_serial_port()
{
	serial_set_baud_rate(USB_COMM, BAUD_RATE);
	serial_receive_ring(USB_COMM, receive_buffer, sizeof(receive_buffer));
}

void check_for_new_bytes_received()
{
	while(serial_get_received_bytes(USB_COMM) != receive_buffer_position)
	{
		process_received_byte(receive_buffer[receive_buffer_position]);

		if (receive_buffer_position == sizeof(receive_buffer) - 1)
			receive_buffer_position = 0;
		else
			receive_buffer_position++;
	}
}

void process_received_byte(char byte)
{
	switch (byte)
	{
		case 'Z':
		case 'z':
			command_type = COMMAND_TYPE_ZERO;
			break;

		case 'P':
		case 'p':
			command_type = COMMAND_TYPE_PRINT;
			break;

		case 'T':
		case 't':
			command_type = COMMAND_TYPE_TOGGLE;
			break;

		case 'R':
		case 'r':
			command_color = COMMAND_COLOR_RED;
			break;

		case 'G':
		case 'g':
			command_color = COMMAND_COLOR_GREEN;
			break;

		case 'Y':
		case 'y':
			command_color = COMMAND_COLOR_YELLOW;
			break;

		case 'A':
		case 'a':
			command_color = COMMAND_COLOR_ALL;
			break;

		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			command_time_ms[command_time_ms_position] = byte;
			command_time_ms_position++;
			break;

		case '\r':
			process_command(command_type, command_color, atoi(command_time_ms));
			command_type = COMMAND_TYPE_NONE;
			command_color = COMMAND_COLOR_NONE;
			memset(command_time_ms, 0, sizeof(command_time_ms));
			command_time_ms_position = 0;
			break;

		default:
			break;
	}

	wait_for_sending_to_finish();
	send_buffer[0] = byte;
	serial_send(USB_COMM, send_buffer, 1);
}

void process_command(char type, char color, int time_ms)
{
	switch (type)
	{
		case COMMAND_TYPE_PRINT:
			if ((color == COMMAND_COLOR_GREEN) || (color == COMMAND_COLOR_ALL))
			{
				memset(send_buffer, 0, sizeof(send_buffer));
				sprintf(send_buffer, "\r\nG: %lu green\r\n", count_green_toggle);
				wait_for_sending_to_finish();
				serial_send(USB_COMM, send_buffer, sizeof(send_buffer));
				wait_for_sending_to_finish();
				//memset(send_buffer, 0, sizeof(send_buffer));
			}
			if ((color == COMMAND_COLOR_YELLOW) || (color == COMMAND_COLOR_ALL))
			{
				memset(send_buffer, 0, sizeof(send_buffer));
				sprintf(send_buffer, "\r\nY: %lu yellow\r\n", count_yellow_toggle);
				wait_for_sending_to_finish();
				serial_send(USB_COMM, send_buffer, sizeof(send_buffer));
				wait_for_sending_to_finish();
				//memset(send_buffer, 0, sizeof(send_buffer));
			}
			if ((color == COMMAND_COLOR_RED) || (color == COMMAND_COLOR_ALL))
			{
				memset(send_buffer, 0, sizeof(send_buffer));
				sprintf(send_buffer, "\r\nR: %lu red\r\n", count_red_toggle);
				wait_for_sending_to_finish();
				serial_send(USB_COMM, send_buffer, sizeof(send_buffer));
				wait_for_sending_to_finish();
				//memset(send_buffer, 0, sizeof(send_buffer));
			}
			break;

		case COMMAND_TYPE_TOGGLE:
			if ((color == COMMAND_COLOR_GREEN) || (color == COMMAND_COLOR_ALL))
			{
				if (time_ms == 0)
				{
					TCCR1A = 0x00;
					TCCR1A = 0x00;
				}
				else
				{
					TCCR1A = 0x82;
					TCCR1B = 0x1D;
				}
				float top = time_ms * pwm_counts_per_ms;
				ICR1 = (int)top;
				OCR1A = (int)(top / 2.0);
			}
			if ((color == COMMAND_COLOR_YELLOW) || (color == COMMAND_COLOR_ALL))
			{
				interrupts_per_toggle_16_bit = time_ms / 200;
				lcd_goto_xy(8, 0);
				print_unsigned_long(time_ms);
			}
			if ((color == COMMAND_COLOR_RED) || (color == COMMAND_COLOR_ALL))
			{
				interrupts_per_toggle_8_bit = time_ms / 2;
				lcd_goto_xy(8, 1);
				print_unsigned_long(time_ms);
			}

			memset(send_buffer, 0, sizeof(send_buffer));
			sprintf(send_buffer, "\r\n");
			wait_for_sending_to_finish();
			serial_send(USB_COMM, send_buffer, sizeof(send_buffer));
			break;

		case COMMAND_TYPE_ZERO:
			if ((color == COMMAND_COLOR_GREEN) || (color == COMMAND_COLOR_ALL))
			{
				count_green_toggle = 0;
			}
			if ((color == COMMAND_COLOR_YELLOW) || (color == COMMAND_COLOR_ALL))
			{
				count_yellow_toggle = 0;
			}
			if ((color == COMMAND_COLOR_RED) || (color == COMMAND_COLOR_ALL))
			{
				count_red_toggle = 0;
			}

			memset(send_buffer, 0, sizeof(send_buffer));
			sprintf(send_buffer, "\r\n");
			wait_for_sending_to_finish();
			serial_send(USB_COMM, send_buffer, sizeof(send_buffer));
			break;
	}
}

void wait_for_sending_to_finish()
{
	unsigned long timeout = 100;
	while (!serial_send_buffer_empty(USB_COMM) && (timeout-- > 0))
		serial_check();
}

void configure_8_bit_timer()
{
	// Timer 0 is going to be set into Compare Output mode, non-PWM mode.
	// Compare output mode: We'd like to Clear OC0A on Compare Match (0b10)
	// and leave OC0B alone, meaning disconnected (0b00).
	// COM0A1 = 1, COM0A0 = 0, COM0B1 = 0, COM0B0 = 0
	// We need to set the timer into Clear Timer on Compare Match (CTC) mode
	// WGM2 = 0, WGM1 = 1, WGM0 = 0
	// The closest we can get to 1 interrupt every ms is by using a prescaler
	// of 256.  CS02 = 1, CS01 = 0, CS00 = 0
	TCCR0A = 0x82;
	TCCR0B = 0x04;
	// With a prescaler of 256, we can enter 78 (0x4E) as our OCR0A and get an
	// interrupt every 0.9984 ms.  Not too shabby.
	OCR0A = 0x4E;
	TIMSK0 = 0x02;
}

void configure_pwm()
{
	// To get a 1 second LED toggle, you need a 256 scalar
	// with a 0x4E top thingy.
	DDRD |= 0x20;
	TCCR1A = 0x82;
	TCCR1B = 0x1D;
	TIMSK1 = 0x03;

	// Top of the saw tooth
	//	ICR1 = 0x1000;
	//ICR1 = 19531;
	ICR1 = 500 * pwm_counts_per_ms;

	// The spot on the saw tooth where you toggle the output
	//	OCR1A = 0x0400;
	//OCR1A = 9766;
	OCR1A = 250 * pwm_counts_per_ms;
}

void configure_16_bit_timer()
{
	TCCR3A = 0x80;
	TCCR3B = 0x1B;
	TCCR3C = 0x00;
	ICR3 = 0x7A12;
	TIMSK3 = 0x02;
}

unsigned long start_ticks_8_bit = 0;
unsigned long end_ticks_8_bit = 0;
unsigned long interrupt_counter_8_bit = 0;
// 8-bit interrupt handler
ISR(TIMER0_COMPA_vect)
{
	if (interrupts_per_toggle_8_bit > 0)
	{
		interrupt_counter_8_bit++;
		if((interrupt_counter_8_bit % interrupts_per_toggle_8_bit) == 0)
		{
			lcd_goto_xy(0, 0);
			end_ticks_8_bit = get_ticks();
			print_unsigned_long(ticks_to_microseconds(end_ticks_8_bit - start_ticks_8_bit));
			start_ticks_8_bit = end_ticks_8_bit;

			interrupt_counter_8_bit = 0;

			release_red_led_task = true;
		}
	}
	else
	{
		set_digital_output(IO_A2, LOW);
	}
}

ISR(TIMER1_COMPA_vect)
{
	// x-ms busy-wait for-loop, problem #3
	//sei();
	//for(int i = 0; i < 51; i++)
	//{
		//WAIT_10MS;
	//}
	count_green_toggle++;
}

ISR(TIMER1_OVF_vect)
{
	// x-ms busy-wait for-loop, problem #3
	//sei();
	//for(int i = 0; i < 51; i++)
	//{
		//WAIT_10MS;
	//}
	count_green_toggle++;
}

unsigned long start_ticks_16_bit = 0;
unsigned long end_ticks_16_bit = 0;
unsigned long interrupt_counter_16_bit = 0;
// 16-bit interrupt handler
ISR(TIMER3_COMPA_vect)
{
	sei();
	for(int i = 0; i < 51; i++)
	{
		WAIT_10MS;
	}
	if(interrupts_per_toggle_16_bit > 0)
	{
		interrupt_counter_16_bit++;
		if ((interrupt_counter_16_bit % interrupts_per_toggle_16_bit) == 0)
		{
			lcd_goto_xy(0, 1);
			end_ticks_16_bit = get_ticks();
			print_unsigned_long(ticks_to_microseconds(end_ticks_16_bit - start_ticks_16_bit));
			start_ticks_16_bit = end_ticks_16_bit;

			interrupt_counter_16_bit = 0;

			set_digital_output(IO_A0, TOGGLE);
			count_yellow_toggle++;
		}
	}
	else
	{
		set_digital_output(IO_A0, LOW);
	}
}