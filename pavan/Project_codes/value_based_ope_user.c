#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include<string.h>

int main()
{

	int fd = open("/dev/led_gpio", O_RDWR);

	if (fd < 0)
	{
		perror("open");
		return -1;
	}

	char val[20];
	printf("Enter the LDR valuen:");
	scanf("%s",val);

	if(write(fd,&val,strlen(val)+1) != 1)
	{
		perror("write");
		return -1;
	}


	// Read GPIO status
	if (read(fd, &val,10) != 1)
	{
		perror("read");
		close(fd);
		return -1;
	}
	printf("Value of LDR = %s",val);

	close(fd);
	return 0;

}

