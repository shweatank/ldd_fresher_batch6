#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>

struct array_t {
        int arr[50];
	int size;
};

static struct array_t req;

int main(int argc, char *argv[])
{
	int fd;

	fd = open("/dev/sort_num_driver", O_RDWR);
	if(fd < 0)
	{
		perror("open");
		return 1;
	}
	
	printf("Enter the size of an array (max : 50) : ");
	scanf("%d",&req.size);

	printf("Enter the array elements \n");
	for(int i = 0 ; i < req.size ; i++)
	{
		scanf("%d",&req.arr[i]);
	}

	write( fd, &req, sizeof( struct array_t ));
	read( fd, &req, sizeof( struct array_t ));

	printf("Sorted array received from driver :\n    -->  ");
	for(int i = 0; i < req.size; i++)
		printf("%d ", req.arr[i]);

	printf("\n");
	close(fd);
	return 0;
}
