
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>
#include <linux/types.h>

#define TCN75_SLAVEADDR	0x4c

int i2c_read(int fd, unsigned char reg, unsigned char *data, unsigned short len)
{
	write(fd, &reg, 1);
	read(fd, data, len);

	return 0;
}

float tcn75_convertTemp(unsigned char aa, unsigned char bb)
{
	float temp;

	if( aa > 127 )
	{
		temp = ( (aa-128) * -1);
		if( bb == 128 )
		{
			temp -= 0.5;
		}
	} else
	{
		temp = aa;
		if( bb == 128 )
		{
			temp += 0.5;
		}
	}

	return temp;
}

int main(int argc, char *argv[])
{
	int fd;

	if ((fd = open(argv[1], O_RDWR)) < 0)
	{
		printf("Failed to open the i2c bus %s", argv[1]);
		exit(1);
	}

	if (ioctl(fd, I2C_SLAVE, TCN75_SLAVEADDR) < 0)
	{
		printf("Failed to acquire bus access and/or talk to slave.\n");
		exit(1);
	}

	while(1)
	{
		unsigned char data[2];

		i2c_read(fd, 0, data, 2);

		printf("<%02X(%d)><%02X(%d)> ==> %f\n\r", data[0], data[0], data[1], data[1], tcn75_convertTemp(data[0], data[1]));
		sleep(1);
	}

	close(fd);

	return 0;
}

// end of file

