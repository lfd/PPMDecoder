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

#include <avr/io.h>
#include <avr/interrupt.h>

#include "ppm.h"

#define TIMEOUT 0xea5f /* 3ms at 20Mhz	=> 60000 ticks */

/* values needed for scaling:

				min trim:	|	mid trim:	|	max trim:
	min stick pos.		19717			22097			24478		
	max stick pos.		35717			38096			40476
		
 */
#define TICKS_MIN 22097UL
#define TICKS_MAX 38096UL

#define PPM_MIN 978
#define PPM_MAX 2017


static uint16_t ppm_rc[CHANNELS];
static uint16_t ppm_fallback[CHANNELS] = {
	[ 0 ... CHANNELS-2 ] = PPM_MIN,
	[       CHANNELS-1 ] = PPM_MAX,
};

volatile uint16_t *ppm_data = ppm_fallback;

static int channel;

/* variables to store ticks */
uint16_t ticks;
uint16_t ticks_previous;
uint16_t ticks_result;

/* Initialise AVR ATmega88 timer 1*/
void ppm_init(void)
{	
	/* no prescaler */
	TCCR1B  = (1 << CS10);
	/* enable noise chanceler */
	TCCR1B |= (1 << ICNC1);
	/* select positive input capture edge */
	TCCR1B |= (1 << ICES1);
	/* enable input capture interrupt */
	TIMSK1  = (1 << ICIE1);
	/* enable output compare a interrupt */
	TIMSK1 |= (1 << OCIE1A);

	channel = -1;
}

/* Interrupt: detect positive edge on ICP1 */
ISR(TIMER1_CAPT_vect)
{
	ticks = ICR1;
	OCR1A = ticks + TIMEOUT;

	if(ticks > ticks_previous)
		ticks_result = ticks - ticks_previous;
	/* timer1 overflow */
	else
		ticks_result = ticks + (0xffff - ticks_previous) + 1;

	ticks_previous = ticks;

	if(channel > -1 && channel < CHANNELS) {
		ppm_rc[channel] = scale(ticks_result);
	}

	channel++;

	/* if more channels are detected than we actually have, we need to send fallback values */
	if (CHANNELS > 40)
		ppm_data = ppm_fallback;
	else if (channel == CHANNELS-1)
		ppm_data = ppm_rc;
}

/* Interrupt: timeout (indicates next ppm frame or lost connection) */
ISR(TIMER1_COMPA_vect)
{	
	static unsigned int timeout_ctr = 0;

	if (channel == -1)
		timeout_ctr++;
	else
		timeout_ctr = 0;

	/* if we get 10 or more timeouts consecutively, we lost remote connection */
	if (timeout_ctr == 10)
		ppm_data = ppm_fallback;

	channel = -1;
}

/* apply scaling to ppm_data */
inline uint16_t scale(const uint16_t ticks)
{	
	if (ticks < TICKS_MIN)
 		return PPM_MIN;

	if (ticks > TICKS_MAX)
		return PPM_MAX;

	return (((ticks - TICKS_MIN) * (PPM_MAX - PPM_MIN))
		/ (TICKS_MAX - TICKS_MIN)) + PPM_MIN;
}

