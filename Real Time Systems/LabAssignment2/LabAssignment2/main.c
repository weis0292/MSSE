/* LabAssignment2 - an application for the Pololu Orangutan SVP
 *
 * This application uses the Pololu AVR C/C++ Library.  For help, see:
 * -User's guide: http://www.pololu.com/docs/0J20
 * -Command reference: http://www.pololu.com/docs/0J18
 *
 * Created: 3/9/2015 8:33:21 PM
 *  Author: WeispfeM
 */

// Here is how to use the commands.  Please note that all commands
// must be followed by a 'return' to execute them.  This allows you
// to make a mistake and simply type the command you meant without
// any issue.  The program will echo any valid character you type
// and not echo invalid characters.
//
// Logging - 'l' or 'L'  This command will begin logging on the first
//		press and stop logging and print the values on the second press
// Print - 'v' or 'V' This will simply print to the serial port the
//		values requested in the lab, and possibly some additional
// Position - 'rxxx' or 'Rxxx' where xxx is the desired position in counts
// Speed - 'sxxx' or 'Sxxx' where xxx is the desired velocity in counts per 10ms
// Proportional Gain - 'P' to increment the proportional gain
//		'p' to decrement the proportional gain
// Integral Gain - 'P' to increment the integral gain
//		'p' to decrement the integral gain
// Derivative Gain - 'D' to increment the derivative gain
//		'd' to decrement the derivative gain
// Trajectory - 't' or 'T' to initiate the pre-programmed trajectory

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <pololu/orangutan.h>

#include "lcd.h"
#include "motor.h"
#include "pc_interrupt.h"
#include "pid.h"
#include "timer_1.h"
#include "serial.h"

bool process_byte(char byte);
void process_command(char type, int v);
void print_motor_information();
void print_pid_values();
void print_new_line();
void reset_command_pos_vel();
bool g_print_values = false;
void print_logged_values();

#define KP_CHANGE 0.2
#define KI_CHANGE 0.01
#define KD_CHANGE 1

char command_pos_vel[6];
int command_pos_vel_position = 0;

int main()
{
	lcd_init();
	configure_serial_port(&process_byte);
	motor_init();
	pc_interrupt_init();
	pid_init();
	timer_1_init();

	while(true)
	{
		serial_check();
		check_for_new_bytes_received();

		if (g_release_print_task)
		{
			g_release_print_task = false;
			print_motor_information();
		}
		if (g_release_log_values_ready)
		{
			g_release_log_values_ready = false;
			print_logged_values();
		}
	}
}

#define COMMAND_TYPE_NONE 'N'
#define COMMAND_TYPE_LOGGING 'L'
#define COMMAND_TYPE_VIEW 'V'
#define COMMAND_TYPE_POSITION 'R'
#define COMMAND_TYPE_SPEED 'S'
#define COMMAND_TYPE_INC_KP 'P'
#define COMMAND_TYPE_DEC_KP 'p'
#define COMMAND_TYPE_INC_KI 'I'
#define COMMAND_TYPE_DEC_KI 'i'
#define COMMAND_TYPE_INC_KD 'D'
#define COMMAND_TYPE_DEC_KD 'd'
#define COMMAND_TYPE_TRAJECTORY 'T'

char command_type = COMMAND_TYPE_NONE;

bool process_byte(char byte)
{
	bool valid_char = true;

	switch (byte)
	{
		case 'L':
		case 'l':
		command_type = COMMAND_TYPE_LOGGING;
		break;

		case 'V':
		case 'v':
			// View the current values Kd, Kp, Vm, Pr, Pm, and T
			command_type = COMMAND_TYPE_VIEW;
		break;

		case 'R':
		case 'r':
		// Set the reference position in counts
		command_type = COMMAND_TYPE_POSITION;
		reset_command_pos_vel();
		break;

		case 'S':
		case 's':
		// Set the reference speed in counts per second
		command_type = COMMAND_TYPE_SPEED;
		reset_command_pos_vel();
		break;

		case 'P':
		// Increase Kp by an amount of your choice
		command_type = COMMAND_TYPE_INC_KP;
		break;

		case 'p':
		// Decrease Kp by an amount of your choice
		command_type = COMMAND_TYPE_DEC_KP;
		break;

		case 'I':
		// Increase Ki by an amount of your choice
		command_type = COMMAND_TYPE_INC_KI;
		break;

		case 'i':
		// Decrease Ki by an amount of your choice
		command_type = COMMAND_TYPE_DEC_KI;
		break;

		case 'D':
		// Increase Kd by an amount of your choice
		command_type = COMMAND_TYPE_INC_KD;
		break;

		case 'd':
		// Decrease Kd by an amount of your choice
		command_type = COMMAND_TYPE_DEC_KD;
		break;

		case 'T':
		case 't':
		// Execute trajectory
		command_type = COMMAND_TYPE_TRAJECTORY;
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
		command_pos_vel[command_pos_vel_position] = byte;
		command_pos_vel_position++;
		valid_char = ((command_type == COMMAND_TYPE_POSITION) || (command_type == COMMAND_TYPE_SPEED));
		break;
		
		case ' ':
		valid_char = ((command_type == COMMAND_TYPE_POSITION) || (command_type == COMMAND_TYPE_SPEED));
		break;

		case '\r':
		process_command(command_type, atoi(command_pos_vel));
		command_type = COMMAND_TYPE_NONE;
		reset_command_pos_vel();
		break;
		
		default:
		valid_char = false;
		break;
	}
	
	return valid_char;
}

void reset_command_pos_vel()
{
	memset(command_pos_vel, 0, sizeof(command_pos_vel));
	command_pos_vel_position = 0;
}

void process_command(char type, int v)
{
	switch(type)
	{
		case COMMAND_TYPE_LOGGING:
		if (g_log_values)
		{
			turn_off_logging();
		}
		else
		{
			turn_on_logging();
		}
		break;
		
		case COMMAND_TYPE_VIEW:
		print_pid_values();
		break;
		
		case COMMAND_TYPE_POSITION:
		set_pid_for_position();
		set_desired_position(v);
		print_new_line();
		break;
		
		case COMMAND_TYPE_SPEED:
		set_pid_for_velocity();
		set_desired_velocity(v);
		print_new_line();
		break;
		
		case COMMAND_TYPE_INC_KP:
		g_pid.pGain += KP_CHANGE;
		print_pid_values();
		break;

		case COMMAND_TYPE_DEC_KP:
		g_pid.pGain -= KP_CHANGE;
		print_pid_values();
		break;
		
		case COMMAND_TYPE_INC_KI:
		g_pid.iGain += KI_CHANGE;
		print_pid_values();
		break;
		
		case COMMAND_TYPE_DEC_KI:
		g_pid.iGain -= KI_CHANGE;
		print_pid_values();
		break;
		
		case COMMAND_TYPE_INC_KD:
		g_pid.dGain += KD_CHANGE;
		print_pid_values();
		break;
		
		case COMMAND_TYPE_DEC_KD:
		g_pid.dGain -= KD_CHANGE;
		print_pid_values();
		break;
		
		case COMMAND_TYPE_TRAJECTORY:
		execute_trajectory();
		break;
	}
}

void print_pid_values()
{
	memset(send_buffer, 0, sizeof(send_buffer));
	sprintf(send_buffer, "\r\nKp: %0.1f\r\nKi: %0.2f\r\nKd: %ld\r\nDesired Position: %ld\r\nActual Position: %ld\r\nDesired Velocity: %ld\r\nActual Velocity: %ld\r\nTorque: %d\r\n",
		g_pid.pGain, g_pid.iGain, g_pid.dGain, g_desired_position, g_counts_m1, g_desired_velocity, g_velocity, g_torque);
	wait_for_sending_to_finish();
	serial_send(USB_COMM, send_buffer, sizeof(send_buffer));
	wait_for_sending_to_finish();
}

void print_new_line()
{
	memset(send_buffer, 0, sizeof(send_buffer));
	sprintf(send_buffer, "\r\n");
	wait_for_sending_to_finish();
	serial_send(USB_COMM, send_buffer, sizeof(send_buffer));
	wait_for_sending_to_finish();
}

void print_motor_information()
{
	lcd_goto_xy(0, 0);
	printf("Pos: %10ld", g_counts_m1);
	//printf("Pos: %10ld", time);
	lcd_goto_xy(0, 1);
	printf("Vel: %10ld", g_velocity);
}

void print_logged_values()
{
	print_new_line();

	for(int i = 0; i < g_array_position; i++)
	{
		memset(send_buffer, 0, sizeof(send_buffer));
		// This line is for all tests except the interpolator
		//sprintf(send_buffer, "%lu, %ld, %ld, %ld, %ld\r\n", g_times[i], g_desired_values[i], g_actual_values[i], (g_actual_velocity[i] * 100), g_desired_values[i] - g_actual_values[i]);
		// Which makes the following line for the interpolator
		sprintf(send_buffer, "%lu, %ld, %ld, %ld\r\n", g_times[i], g_desired_values[i], g_actual_values[i], g_actual_torque[i]);
		wait_for_sending_to_finish();
		serial_send(USB_COMM, send_buffer, sizeof(send_buffer));
		wait_for_sending_to_finish();
	}

	memset(send_buffer, 0, sizeof(send_buffer));
	sprintf(send_buffer, "%d\r\n", g_array_position);
	wait_for_sending_to_finish();
	serial_send(USB_COMM, send_buffer, sizeof(send_buffer));
	wait_for_sending_to_finish();
}