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

void uart_set_recv_handler(void (*handler)(unsigned char c));

void uart_init(void);

void uart_putc(const char c);
void uart_puts(const char* str);
