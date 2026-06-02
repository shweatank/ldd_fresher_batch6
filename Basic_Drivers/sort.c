#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define DEVICE "/dev/sort_driver"

struct Operation
{
	int size;
	int arr[];
};
int main() {
	int fd;
	int read_buff;
	int n;
        printf("Enter the Size\n");
	scanf("%d",&n);
	struct Operation *op =malloc(sizeof(struct Operation)+n*sizeof(int));
	op->size = n;
	for(int i=0;i<n;i++)
	{
		scanf("%d",&op->arr[i]);
	}
	fd = open(DEVICE, O_RDWR);
	if (fd == -1) {
		perror("Failed to open device");
		return -1;
	}
	write(fd,op,sizeof(struct Operation)+n*sizeof(int));
	lseek(fd, 0, SEEK_SET);
	read(fd,op,sizeof(struct Operation)+n*sizeof(int));
        printf("Result from driver: ");
	for(int i=0;i<n;i++)
        {
                printf("%d ",op->arr[i]);
        }
        printf("\n");
	free(op);
	close(fd);
	return 0;
}
