#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

struct bike{
int mode;
int speed;
char name[20];
};

#define IOCTL_MAGIC 'C'
#define IOCTL_BIKE_DATA _IOWR(IOCTL_MAGIC,1,struct bike)

int main(){

	int fd;
	fd=("/dev/bike",O_RDWR);
	if(fd<0){
	perror("open");
	return 0;
	}
	
	struct bike b;
	printf("enter the mode number , speed , name of bike\n");
	scanf(" %d %d %s",&b.mode,&b.speed,b.name);
	ioctl(fd,IOCTL_BIKE_DATA,&b);
	printf("Reseted values\n mode =%d\n speed =%d\n,name=%s\n",b.mode,b.speed,b.name);
}
