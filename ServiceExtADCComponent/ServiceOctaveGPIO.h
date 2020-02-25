/*
 * ServiceOctaveGPIO.h
 *
 *  Created on: Oct 3, 2019
 *     Project: ServiceExtADC
 *      Author: Bartosz Cieslicki
 *      E-Mail: bartek@energiya.pl
 *         Tel: +48 505 148 375
 */

#ifndef SERVICEOCTAVEGPIO_H_
#define SERVICEOCTAVEGPIO_H_

#include "interfaces.h"

struct GPIO
{
	uint8_t GPIO_NO;
	char Path[50];
	uint8_t Value;
	void* EventRef;
	void* IoHandlerRef;
};

struct GPIO GPIOS[8];
io_NumericPushHandlerRef_t AddressHandlerGPIO_I2C;

#endif /* SERVICEOCTAVEGPIO_H_ */
