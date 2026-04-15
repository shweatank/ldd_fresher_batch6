#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>

struct palindrome_t {
    char buf[128];
    int status;
};

static struct palindrome_t req;
 
#define IOCTL_MAGIC 'A'
#define PALINDROME_  _IOWR(IOCTL_MAGIC, 1, struct palindrome_t)

int main()
{
	
	int fd;

	printf("Enter the string\n");
	fgets(req.buf,128,stdin);

	if(req.buf[strlen(req.buf)-1] == '\n')
	      req.buf[strlen(req.buf)-1] = '\0';

	fd = open("/dev/palindrome_driver", O_RDWR);
	if(fd < 0)
	{
		perror("open");
		return 1;
	}

	  ioctl(fd, PALINDROME_, &req);
	  if(req.status == 0)
	      printf("String is Not a Palindrome\n");
	  else
	      printf("String is Palindrome\n");
	

	close(fd);
	return 0;
}
