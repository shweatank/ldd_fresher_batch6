#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>

struct CASE_CONVERSION {
        char buf[128];
};

static struct CASE_CONVERSION case_cnv_req;


int main()
{
	
	int fd;

	printf("Enter the string\n");
	
	fgets(case_cnv_req.buf,128,stdin);

	if(case_cnv_req.buf[strlen(case_cnv_req.buf)-1] == '\n')
	      case_cnv_req.buf[strlen(case_cnv_req.buf)-1] = '\0';

	fd = open("/dev/case_conv_driver", O_RDWR);

	if(fd < 0)
	{
		perror("open");
		return 1;
	}

	write(fd, &case_cnv_req, sizeof( struct CASE_CONVERSION ));
	read(fd, &case_cnv_req, sizeof( struct CASE_CONVERSION ));

	printf("User: got back %s from kernel\n",case_cnv_req.buf);
	

	close(fd);
	return 0;
}
