/* This is an application program that interacts with a lm75 driver.
 * If it's executed without parametrs, it will display temperature and exit.
 * If there is a parameter, the program will enter an infinite loop
 * that can be stopped with ^C.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define TEMPERATUREFILE \
	"/sys/bus/i2c/devices/2-004c/hwmon/hwmon0/temp1_input"

int main(int argc, char *argv[])
{
	int loop = 0;
	char buf[8];
	int f;
	int n;
	int val, vali, valf;

	loop = atoi(argv[1]);

	f = open(TEMPERATUREFILE, 0);
	if (f < 0) {
		fprintf(stderr, "Failed to open the file\n");
		return -1;
	}

	while (loop > 0) {
		lseek(f, 0, SEEK_SET);
		n = read(f, buf, 8);
		if (n >=0 && n < 8)
			buf[n] = 0;
		valf = atoi(buf);
		vali = valf / 10;
		valf %= 10;
		printf("temperature = %d.%d\n", vali, valf);

		loop--;
		sleep(1);
	}

	close(f);

	return 0;
}

