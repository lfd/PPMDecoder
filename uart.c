/*
 * PPMDecoder -  Decode PPM signals on an AVR and convert it to I2C / UART
 *
 * Copyright (c) OTH Regensburg, 2016
 *
 * Authors:
 *  Ralf Ramsauer <ralf.ramsauer@oth-regensburg.de>
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 */

#include <stddef.h>
#include <avr/interrupt.h>
#include <avr/io.h>

#include "uart.h"

#define UBRR_VAL ((F_OSC+UART_BAUD*8)/(UART_BAUD*16)-1)
#define BAUD_REAL (F_OSC/(16*(UBRR_VAL+1)))
#define BAUD_ERROR ((BAUD_REAL*1000)/UART_BAUD)

#if ((BAUD_ERROR<985) || (BAUD_ERROR>1010))
  #warn BAUD_ERROR
  #error Choose another crystal. Baud error too high.
#endif

/* Register redefinition for mega88 */
#ifndef UDR
#define USART_RXC_vect USART_RX_vect
#define UDR    UDR0
#define UCSRA  UCSR0A
# define UDRE  UDRE0
#define UCSRB  UCSR0B
# define RXEN  RXEN0
# define TXEN  TXEN0
# define RXCIE RXCIE0
#define UCSRC  UCSR0C
# define UCSZ0 UCSZ00
# define UCSZ1 UCSZ01
#define UBRRL  UBRR0L
#define UBRRH  UBRR0H
#endif

static void (*recv_handler)(unsigned char c) = NULL;

ISR(USART_RXC_vect)
{
	char c = UDR;
	if(recv_handler) {
		cli();
		recv_handler(c);
		sei();
	}
}

void uart_init()
{
	// Enable receive and transmit
	UCSRB = (1<<RXEN)|(1<<TXEN)|(1<<RXCIE);

	// Asynchronous mode, 8N1
	UCSRC = (1<<UCSZ1)|(1<<UCSZ0);

	UBRRH = UBRR_VAL >> 8;
	UBRRL = UBRR_VAL & 0xFF;
}

void uart_set_recv_handler(void (*handler)(unsigned char c))
{
	recv_handler = handler;
}

void uart_putc(const char c)
{
	while ( !(UCSRA & (1<<UDRE)) );
	UDR = (const unsigned char)c;
}

void uart_puts(const char* str)
{
	do {
		uart_putc(*str);
	} while(*++str);
}
