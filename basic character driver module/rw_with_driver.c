#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
int main()
{
	int fd = open("/dev/basic_char",O_RDWR);
	if(fd < 0){
	  perror("open");
	}
	printf("file open with fd = %d\n",fd);
	char buff[128] = "I am writing to device driver\n";
        write(fd,buff,strlen(buff));	

	close(fd);

}
