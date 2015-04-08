/*
 * two_wire_interface.h
 *
 * Created: 4/3/2015 10:23:49 AM
 * Author: Finga Gunz
 */

#ifndef TWO_WIRE_INTERFACE_H_
#define TWO_WIRE_INTERFACE_H_

void twi_start(unsigned char address);
void twi_stop();
void twi_write(unsigned char data);
unsigned char twi_read();

#endif /* TWO_WIRE_INTERFACE_H_ */
