#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>


struct array_t
{
        int arr[30];
        int size;
        int sum;
        int avg;
};

#define IOCTL_MAGIC 'A'
#define SUM_AVG_IOCTL  _IOWR(IOCTL_MAGIC, 1, struct array_t)

static struct array_t req;


int main()
{
	
	int fd;

	int n;
	printf("Enter the size of array ( Do not exceed 30) : ");
	scanf("%d",&n);

	printf("Enter array elements\n");
	for(int i = 0 ; i < n; i ++)
	     scanf("%d",&req.arr[i]);
	
	req.size = n;

	fd = open("/dev/array_driver", O_RDWR);
	if(fd < 0)
	{
		perror("open");
		return 1;
	}

	 ioctl(fd, SUM_AVG_IOCTL, &req);
	 printf("Sum of all Elements = %d, Average %d\n", req.sum, req.avg);
	

	close(fd);
	return 0;
}
