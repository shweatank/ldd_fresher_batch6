#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define DEVICE "/dev/calculation_driver"

struct Operation
{
	int num1;
	int num2;
	char opp[4];
};
int main() {
	int fd;
	int read_buff;
	struct Operation operation;
        printf("Enter the Numbers and Operation that you want to perform\n");
	scanf("%d %d %s",&operation.num1,&operation.num2,operation.opp);
	fd = open(DEVICE, O_RDWR);
	if (fd == -1) {
		perror("Failed to open device");
		return -1;
	}
	write(fd,&operation,sizeof(struct Operation));
	lseek(fd, 0, SEEK_SET);
	read(fd, &operation.num1, sizeof(int));
        printf("Result from driver: %d\n",operation.num1);

	close(fd);
	return 0;
}
