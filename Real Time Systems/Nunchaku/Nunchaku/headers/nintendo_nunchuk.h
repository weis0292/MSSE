/*
 * nintendo_nunchuk.h
 *
 * Created: 4/3/2015 10:26:43 AM
 * Author: Finga Gunz
 */

#ifndef NINTENDO_NUNCHUK_H_
#define NINTENDO_NUNCHUK_H_

#include <stdbool.h>

void nunchuck_update_data();
unsigned char nunchuck_get_joystick_x();
unsigned char nunchuck_get_joystick_y();
uint16_t nunchuck_get_accelerometer_x();
uint16_t nunchuck_get_accelerometer_y();
uint16_t nunchuck_get_accelerometer_z();
bool nunchuck_get_button_c();
bool nunchuck_get_button_z();

#endif /* NINTENDO_NUNCHUK_H_ */