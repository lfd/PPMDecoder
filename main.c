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

int main(void) {
	char buffer[128];
	int i;

	cli();

	uart_init();
	uart_puts("Hello, world!\r\n");

	ppm_init();
	sei();

	for(;;) {

		/* dummy code */
		for (i=0;i<CHANNELS;i++) {
			sprintf(buffer, "%u ", *(ppm_data+i));
			uart_puts(buffer);
		}
		uart_puts("\n\r");

		_delay_ms(100);
   	}
}
