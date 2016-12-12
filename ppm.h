/*
 * PPMDecoder -  Decode PPM signals on an AVR and convert it to I2C / UART
 *
 * Copyright (c) OTH Regensburg, 2016
 *
 * Authors:
 *  Tobias Kottwitz <tobias.kottwitz@oth-regensburg.de>
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 */

#define CHANNELS 16

/* ppm high ticks per channel */
extern volatile uint16_t *ppm_data;

/* initialise AVR ATmega88 timer 1 */
void ppm_init(void);

/* apply scaling to ppm_data_rc */
uint16_t scale(uint16_t time);
