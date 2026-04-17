#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>
#include<sys/wait.h>
#include<stdlib.h>
int main()
{
	int fd=open("/dev/char_driver_sort_array",O_WRONLY);
	if(fd<0)
	{
		perror("open");
		exit(0);
	}
	printf("File is opened successfully\n");
	int n,ele;
	printf("Enter the number of elements\n");
	scanf("%d",&ele);
	int *ptr=malloc(ele*sizeof(int));
	printf("Enter the elements\n");
	for(int i=0;i<ele;i++)
	{
		scanf("%d",&ptr[i]);
	}
	printf("\n");

	if(((n=write(fd,ptr,ele*sizeof(int)))<0))
	{
		perror("Write");
		exit(0);
	}
	printf("send\n");
	close(fd);

	fd=open("/dev/char_driver_sort_array",O_RDONLY);
	if(fd<0)
	{
		perror("open");
		exit(0);
	}
	printf("FIle is opened successfully\n");
	if(((n=read(fd,ptr,ele*sizeof(int))<0)))
	{
		perror("read");
		exit(0);
	}
	for(int i=0;i<ele;i++)
	{
		printf("%d ",ptr[i]);
	}
	printf("\n");

}

