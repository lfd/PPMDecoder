/*
 * PPMDecoder -  Decode PPM signals on an AVR and convert it to I2C / UART
 *
 * Copyright (c) OTH Regensburg, 2016
 *
 * Authors:
 *  Ralf Ramsauer <ralf.ramsauer@oth-regensburg.de>
 *  Tobias Kottwitz <tobias.kottwitz@oth-regensburg.de>
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>

#include "ppm.h"
#include "TWI_slave.h"

#define I2C_DEVICE_ADDRESS 0x70

int main(void) {
	unsigned char i2c_device_address = I2C_DEVICE_ADDRESS;

	cli();

	ppm_init();
	TWI_Slave_Initialise( (unsigned char) ((i2c_device_address << TWI_ADR_BITS) | (1 << TWI_GEN_BIT)) );

	sei();

	TWI_Start_Transceiver();

	for(;;) {
		while (TWI_Transceiver_Busy());
		TWI_Start_Transceiver_With_Data((unsigned char*)ppm_data, 32);
	}
}
