#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main()
{

	int fd = open("/dev/led_gpio", O_RDWR);
	if (fd < 0)
	{
		perror("open");
		return -1;
	}
	char val;

	while(1)
	{
		lseek(fd, 0, SEEK_SET);
		// Read GPIO status
		if (read(fd, &val, 1) != 1)
		{
			perror("read");
			close(fd);
			return -1;
		}

		printf("Status = %c\n", val);

		printf("change the status accordingly:");
		scanf(" %c",&val);

		/*
		// Toggle logic
		if (val == '1')
		val = '0';
		else
		val = '1';
		 */

		// Write back new value
		if (write(fd, &val, 1) != 1)
		{
			perror("write");
			close(fd);
			return -1;
		}

		printf("New Status Written = %c\n", val);
	}
	close(fd);
	return 0;
}
