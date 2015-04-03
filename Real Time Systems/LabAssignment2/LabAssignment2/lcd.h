/*
 * lcd.h
 *
 * Created: 3/9/2015 8:35:24 PM
 *  Author: WeispfeM
 */ 

#ifndef LCD_H_
#define LCD_H_

#include <pololu/orangutan.h>

void lcd_init()
{
	// Clear the LCD screen
	clear();

	// Allow the screen to output results of printf
	lcd_init_printf();
}

#endif /* LCD_H_ */