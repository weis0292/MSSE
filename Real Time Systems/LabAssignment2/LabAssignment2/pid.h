/*
 * pid.h
 *
 * Created: 3/16/2015 9:26:10 PM
 *  Author: WeispfeM
 */ 

#ifndef PID_H_
#define PID_H_

typedef struct
{
	long dState;
	long iState;
	long iMax;
	long iMin;
	double iGain;
	double pGain;
	long dGain;
} SPid;

bool g_log_values = false;
bool g_release_log_values_ready = false;
long g_desired_values[700];
long g_actual_values[700];
long g_actual_velocity[700];
unsigned long g_times[700];
long g_actual_torque[700];
unsigned long g_log_start_time = 0;
int g_array_position = 0;
SPid g_pid;

bool g_is_performing_trajectory = false;
int g_trajectory_count = 3;
long g_trajectory_positions[3] = { 562, -1687, -1656 };
int g_current_trajectory_position = 0;
int g_in_position_counter = 0;
int g_in_position_count = 10;
int g_max_in_position_error = 5;
bool g_motor_in_position = false;

void pid_init()
{
	g_pid.iMax = 1000;
	g_pid.iMin = -1000;

	TCCR3A = 0x80;
	TCCR3B = 0x1A; // prescalar = 8
	TCCR3C = 0x00;
	ICR3 = 25000;//0x61A8; // 25,000
	TIMSK3 = 0x02;
	sei();
}

double update_pid(SPid * pid, long error, long current)
{
	double p_term;
	double i_term;
	double d_term;

	p_term = pid->pGain * error;

	pid->iState += error;
	if (pid->iState > pid->iMax)
	{
		pid->iState = pid->iMax;
	}
	else if (pid->iState < pid->iMin)
	{
		pid->iState = pid->iMin;
	}
	i_term = pid->iGain * pid->iState;

	// The second portion of the below calculation is
	// calculating the acceleration for position control
	// and the rate of change of acceleration for the
	// speed control.  In the confines of this method,
	// it doesn't matter if we're speed control or position
	// control, we just calculate the change in value
	d_term = pid->dGain * (pid->dState - current);
	pid->dState = current;

	// We're allowed to add the d_term here because of the way
	// we're calculating it above.  Because we're doing the
	// previous value minus the current value, we get the negative
	// of the actual change, which allows us to add the value at
	// at this point.
	return p_term + i_term + d_term;
}

volatile unsigned long g_last_counts_m1 = 0;
volatile long g_velocity = 0;
volatile unsigned long g_desired_position = 0;
volatile unsigned long g_desired_velocity = 0;
volatile unsigned long lastticks  = 0;
volatile unsigned long time = 0;
bool g_is_pid_for_position = true;
volatile int g_torque = 0;

void set_desired_position(long desired_position)
{
	g_desired_position = desired_position;
}

void set_desired_velocity(long desired_velocity)
{
	g_desired_velocity = desired_velocity;
}

void set_pid_for_position()
{
	g_desired_position = 0;
	g_is_pid_for_position = true;
}

void set_pid_for_velocity()
{
	g_desired_velocity = 0;
	g_is_pid_for_position = false;
}

void turn_on_logging()
{
	g_array_position = 0;
	g_log_start_time = get_ms();
	g_log_values = true;
}

void turn_off_logging()
{
	g_log_values = false;
	g_release_log_values_ready = true;
}

void execute_trajectory()
{
	set_pid_for_position();
	g_is_performing_trajectory = true;
	g_in_position_counter = 0;
	set_desired_position(g_trajectory_positions[g_current_trajectory_position]);
	g_current_trajectory_position++;
}

// Change this value to change the PID update speed.
volatile int pid_divider = 1;
volatile unsigned long pid_counter = 0;

unsigned long g_time_in_position_us = 0;
unsigned long g_last_time_in_position_ticks = 0;
unsigned long g_time_before_next_move_us = 500000; // 500 milliseconds

// This should fire around every 10 milliseconds
ISR (TIMER3_COMPA_vect)
{
	// Calculate the velocity.  This doesn't need a time
	// element because our loop is at a constant 10ms.

	pid_counter++;
	if ((pid_counter % pid_divider) == 0)
	{
		pid_counter = 0;

		g_velocity = (g_counts_m1 - g_last_counts_m1);

		if(g_is_pid_for_position)
		{
			long error = g_desired_position - g_counts_m1;
			g_torque = fmax(-0xFF, fmin(0xFF, (int)update_pid(&g_pid, error , g_counts_m1)));

			if (error < g_max_in_position_error)
			{
				g_in_position_counter++;
				if (g_in_position_counter >= g_in_position_count)
				{
					if (!g_motor_in_position)
					{
						g_last_time_in_position_ticks = get_ticks();
					}
					g_motor_in_position = true;
					unsigned long ticks = get_ticks();
					g_time_in_position_us += ticks_to_microseconds(ticks - g_last_time_in_position_ticks);
					g_last_time_in_position_ticks = ticks;
					if (g_time_in_position_us >= g_time_before_next_move_us)
					{
						if (g_is_performing_trajectory)
						{
							execute_trajectory();
							if (g_current_trajectory_position == g_trajectory_count)
							{
								g_current_trajectory_position = 0;
								g_is_performing_trajectory = false;
							}
						}
					}
				}
				else
				{
					g_motor_in_position = false;
					g_time_in_position_us = 0;
				}
			}
			else
			{
				g_motor_in_position = false;
				g_time_in_position_us = 0;
			}
		}
		else
		{
			long error = g_desired_velocity - g_velocity;
			g_torque = fmax(-0xFF, fmin(0xFF, g_torque + (int)update_pid(&g_pid, error, g_velocity)));
		}

		set_motor(g_torque);

		g_last_counts_m1 = g_counts_m1;
	}
	
	if (g_log_values)
	{
		g_times[g_array_position] = get_ms() - g_log_start_time;
		g_actual_torque[g_array_position] = g_torque;
		if (g_is_pid_for_position)
		{
			g_desired_values[g_array_position] = g_desired_position;
			g_actual_values[g_array_position] = g_counts_m1;
			g_actual_velocity[g_array_position] = g_velocity;
		}
		else
		{
			g_desired_values[g_array_position] = g_desired_velocity;
			g_actual_values[g_array_position] = g_velocity;
		}
		g_array_position++;
	}

	// Use the code below to verify our PID update rate.
	//unsigned long ticks = get_ticks();
	//time = ticks_to_microseconds(ticks - lastticks);
	//lastticks = ticks;
}

#endif /* PID_H_ */