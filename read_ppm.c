/*
 * read_ppm - Test AVR code from Linux
 *
 * Copyright (c) OTH Regensburg, 2017
 *
 * Authors:
 *  Ralf Ramsauer <ralf.ramsauer@oth-regensburg.de>
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 */

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stropts.h>
#include <string.h>
#include <linux/i2c-dev.h>

#define BUS "/dev/i2c-0"

#define DEVICE_ID 0x70

#define PPM_CHANNELS 16

int main(void)
{
	int fd, i, retval = 1;
	uint16_t ppm_cur[PPM_CHANNELS];
	uint16_t ppm_old[PPM_CHANNELS];

	if ((fd = open(BUS, O_RDWR)) < 0) {
		perror("Failed to open the i2c bus");
		exit(1);
	}

	retval = ioctl(fd, I2C_SLAVE, DEVICE_ID) < 0;
	if (retval) {
		printf("Failed to acquire bus access and/or talk to slave.\n");
		goto out1;
	}

	for(;;) {
		retval = read(fd, ppm_cur, sizeof(ppm_cur));
		if (retval != sizeof(ppm_cur)) {
			printf("Retval = %d :-(\n", retval);
			continue;
		}

		if (memcmp(ppm_old, ppm_cur, sizeof(ppm_cur)) == 0)
			continue;

		memcpy(ppm_old, ppm_cur, sizeof(ppm_cur));

		for (i=0;i<(sizeof(ppm_cur)/sizeof(*ppm_cur));i++) printf("%04d ", ppm_cur[i]);
		printf("   \r");
		fflush(stdout);
	}

	retval = 0;

out1:
	close(fd);
out:
	return retval;
}
