#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
int main()
{
	int fd;
	fd=open("/dev/basic_char",O_WRONLY);
	char write_buff[]="hello_driver";
	write(fd,write_buff,strlen(write_buff)+1);
	printf("Writing in device driver successfully\n");
	close(fd);

	fd=open("/dev/basic_char",O_RDONLY);
	char read_buff[100];
	read(fd,read_buff,sizeof(read_buff));
	printf("Reading in device driver Successfully:%s\n",read_buff);
	close(fd);
	return 0;
}
