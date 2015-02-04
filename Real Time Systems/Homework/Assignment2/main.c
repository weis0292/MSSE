/* Assignment2 - an application for the Pololu Orangutan SVP
 *
 * This application uses the Pololu AVR C/C++ Library.  For help, see:
 * -User's guide: http://www.pololu.com/docs/0J20
 * -Command reference: http://www.pololu.com/docs/0J18
 *
 * Created: 1/30/2015 10:51:35 PM
 *  Author: Mike
 */

#include <pololu/orangutan.h>
#include <stdbool.h>
#include <stdio.h>

const char* _author = "Mike Weispfenning";
const char* _project = "Assignment #2";
const unsigned long _baudRate = 9600;
const int _blinkChangeMS = 25;
const int _firstMask = 0x10;
const int _secondMask = 0x08;

char _receiveBuffer[32];
unsigned char _receiveBufferPosition = 0;
int firstBlinkTime = 250;
int secondBlinkTime = 250;
int *currentBlinkTime = &firstBlinkTime;

void SetupDisplay();
void DisplayVersionInformation();

void SetUpIO();
void PlaySong(bool shouldPlaySong);
void ToggleLEDs();

void SetUpSerialPort();
void CheckForNewBytesReceived();
void ProcessReceivedByte(char byte);
void PrintBlinkSpeeds();

int main()
{
	bool isFirstLEDFlashing = false;
	bool isSecondLEDFlashing = false;
	unsigned long firstTimeMS = 0;
	unsigned long secondTimeMS = 0;

	SetupDisplay();
	DisplayVersionInformation();
	SetUpIO();
	SetUpSerialPort();
	PrintBlinkSpeeds();

	while(true)
	{
		CheckForNewBytesReceived();
		ToggleLEDs(&isFirstLEDFlashing, &firstTimeMS, &isSecondLEDFlashing, &secondTimeMS);
	}
}

void SetupDisplay()
{
	lcd_init_printf();
}

void DisplayVersionInformation()
{
	clear();
	print(_author);
	lcd_goto_xy(0, 1);
	print(_project);
	delay_ms(2000);
	clear();
}

void SetUpIO()
{
	DDRC |= _firstMask;
	DDRD |= _secondMask;
}

void ToggleLEDs(bool *isFirstLEDFlashing, unsigned long *firstTimeMS, bool *isSecondLEDFlashing, unsigned long *secondTimeMS)
{
	unsigned char button = get_single_debounced_button_press(ANY_BUTTON);

	PlaySong(button & MIDDLE_BUTTON);
	if (button & TOP_BUTTON)
	{
		*isFirstLEDFlashing = !*isFirstLEDFlashing;
	}
	if (button & BOTTOM_BUTTON)
	{
		*isSecondLEDFlashing = !*isSecondLEDFlashing;
	}

	unsigned long time = get_ms();
	if (*isFirstLEDFlashing)
	{
		if((time - *firstTimeMS) >= firstBlinkTime)
		{
			PORTC ^= _firstMask;
			*firstTimeMS = time;
		}
	}
	else
	{
		PORTC &= ~_firstMask;
		*firstTimeMS = 0;
	}

	if (*isSecondLEDFlashing)
	{
		if((time - *secondTimeMS) >= secondBlinkTime)
		{
			PORTD ^= _secondMask;
			*secondTimeMS = time;
		}
	}
	else
	{
		PORTD &= ~_secondMask;
		*secondTimeMS = 0;
	}
}

void PlaySong(bool shouldPlaySong)
{
	if (shouldPlaySong)
	{
		play("!T200 L4 V12 MS eeeee8ce8g2ddddd8gf8e8d8g8f8eeeee8ce8g.g8a8a8g8g8f8f8eagfeg8ec8dgc2");
	}
}

void SetUpSerialPort()
{
	serial_set_baud_rate(USB_COMM, _baudRate);
	serial_receive_ring(USB_COMM, _receiveBuffer, sizeof(_receiveBuffer));
}

void CheckForNewBytesReceived()
{
	serial_check();

	while(serial_get_received_bytes(USB_COMM) != _receiveBufferPosition)
	{
		ProcessReceivedByte(_receiveBuffer[_receiveBufferPosition]);

		if (_receiveBufferPosition == sizeof(_receiveBuffer) - 1)
		{
			_receiveBufferPosition = 0;
		}
		else
		{
			_receiveBufferPosition++;
		}
	}
}

void ProcessReceivedByte(char byte)
{
	switch(byte)
	{
		case '+':
			*currentBlinkTime += _blinkChangeMS;
			PrintBlinkSpeeds();
		break;

		case '-':
			*currentBlinkTime -= _blinkChangeMS;
			PrintBlinkSpeeds();
		break;

		// Switch which speed the serial port is modifying
		case 'S':
		case 's':
			currentBlinkTime = (currentBlinkTime == &firstBlinkTime) ? &secondBlinkTime : &firstBlinkTime;
		break;
	}
}

void PrintBlinkSpeeds()
{
	clear();
	printf("Top MS: %4d", firstBlinkTime);
	lcd_goto_xy(0, 1);
	printf("Bot MS: %4d", secondBlinkTime);
}
