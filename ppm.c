/*
 * PPMDecoder -  Decode PPM signals on an AVR and convert it to I2C / UART
 *
 * Copyright (c) OTH Regensburg, 2016
 *
 * Authors:
 *  Tobias Kottwitz <tobias.kottwitz@oth-regensburg.de>
 *  Ralf Ramsauer <ralf.ramsauer@oth-regensburg.de>
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>

#include "ppm.h"

#define MS_TO_TICKS(x) ((uint16_t)(((double)F_OSC/(double)1000)*(double)x))

/* Graupner mx-16 timings */
#define TIMEOUT   MS_TO_TICKS(3)
#define TICKS_MIN MS_TO_TICKS(0.983)
#define TICKS_MAX MS_TO_TICKS(2.025)

#define PPM_MIN 950
#define PPM_MAX 2030
#define PPM_MID (PPM_MIN + (PPM_MAX-PPM_MIN)/2)

/* register names for AVR ATmega8 */
#ifndef TIMSK1
#define TIMSK1 TIMSK
#define ICIE1 TICIE1
#endif

static uint16_t ppm_rc[CHANNELS];
static uint16_t ppm_fallback[CHANNELS] = {	
	PPM_MIN, /* channel1: throttle */
	PPM_MID, /* channel2: pitch */
	PPM_MID, /* channel3: roll */
	PPM_MID, /* channel4: yaw */
	PPM_MIN, /* channel5: */
	PPM_MIN, /* channel6: */
	PPM_MAX, /* channel7: emergency off */
	PPM_MIN, /* channel8: */
	PPM_MIN, /* channel9: */
	PPM_MIN, /* channel10: */
	PPM_MIN, /* channel11: */
	PPM_MIN, /* channel12: */
	PPM_MIN, /* channel13: */
	PPM_MIN, /* channel14: */
	PPM_MIN, /* channel15: */
	PPM_MAX, /* channel16: reserved for failsafe */
};

volatile uint16_t *ppm_data = ppm_fallback;

static int channel;
static unsigned char max_channels = CHANNELS;

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
	if (channel > 40) {
		ppm_data = ppm_fallback;
	} else if (channel == max_channels-1) {
		ppm_data = ppm_rc;
	}
}

/* Interrupt: timeout (indicates next ppm frame or lost connection) */
ISR(TIMER1_COMPA_vect)
{	
	static unsigned int timeout_ctr = 0;
	static unsigned char channels_last_frame = 0;

	if (channel == -1) {
		/* if we get 10 or more timeouts consecutively,
		 * we lost remote connection */
		if (++timeout_ctr == 10) {
			ppm_data = ppm_fallback;
		}
		return;
	}

	if (channels_last_frame == channel && max_channels != channels_last_frame) {
		max_channels = (channels_last_frame > CHANNELS) ?
				CHANNELS : channels_last_frame;
		memcpy(ppm_rc + max_channels, ppm_fallback + max_channels,
		       (CHANNELS - max_channels) * sizeof(*ppm_data));
	}

	channels_last_frame = channel;
	channel = -1;
	timeout_ctr = 0;
}

/* apply scaling to ppm_data */
inline uint16_t scale(const uint16_t ticks)
{	
	if (ticks < TICKS_MIN)
 		return PPM_MIN;

	if (ticks > TICKS_MAX)
		return PPM_MAX;

	return (((uint32_t)(ticks - TICKS_MIN) * (PPM_MAX - PPM_MIN))
		/ (TICKS_MAX - TICKS_MIN)) + PPM_MIN;
}

