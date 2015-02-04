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
unsigned long ManageLED(bool isLEDFlashing, unsigned long timeMS, int blinkTimeMS, void (*toggleLEDMethod)(), void (*turnOffLEDMethod)());
void ToggleFirstLED();
void TurnOffFirstLED();
void ToggleSecondLED();
void TurnOffSecondLED();

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

		unsigned char button = get_single_debounced_button_press(ANY_BUTTON);
		PlaySong(button & MIDDLE_BUTTON);

		bool isTopButtonPressed = button & TOP_BUTTON;
		if(isTopButtonPressed)
		{
			isFirstLEDFlashing = !isFirstLEDFlashing;
		}
		firstTimeMS = ManageLED(isFirstLEDFlashing, firstTimeMS, firstBlinkTime, ToggleFirstLED, TurnOffFirstLED);

		bool isBottomButtonPressed = button & BOTTOM_BUTTON;
		if(isBottomButtonPressed)
		{
			isSecondLEDFlashing = !isSecondLEDFlashing;
		}
		secondTimeMS = ManageLED(isSecondLEDFlashing, secondTimeMS, secondBlinkTime, ToggleSecondLED, TurnOffSecondLED);
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

void PlaySong(bool shouldPlaySong)
{
	if (shouldPlaySong)
	{
		play("!T200 L4 V12 MS eeeee8ce8g2ddddd8gf8e8d8g8f8eeeee8ce8g.g8a8a8g8g8f8f8eagfeg8ec8dgc2");
	}
}

unsigned long ManageLED(bool isLEDFlashing, unsigned long timeMS, int blinkTimeMS, void (*toggleLEDMethod)(), void (*turnOffLEDMethod)())
{
	unsigned long returnTime = timeMS;
	unsigned long time = get_ms();

	if(isLEDFlashing)
	{
		if((time - timeMS) >= blinkTimeMS)
		{
			(*toggleLEDMethod)();
			returnTime = time;
		}
	}
	else
	{
		(*turnOffLEDMethod)();
		returnTime = 0;
	}
	
	return returnTime;
}

void ToggleFirstLED()
{
	PORTC ^= _firstMask;
}

void TurnOffFirstLED()
{
	PORTC &= ~_firstMask;
}

void ToggleSecondLED()
{
	PORTD ^= _secondMask;
}

void TurnOffSecondLED()
{
	PORTD &= ~_secondMask;
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
