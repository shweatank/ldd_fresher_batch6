#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
int main()
{
	struct st
	{
		int num1,num2;
		char op[10];
	};
	struct st obj;
	printf("Enter 5 numbers\n");
	int arr[5],temp[5];
	for(int i=0;i<5;i++)
	scanf("%d",&arr[i]);
	
	int fd=open("/dev/basic_char",O_RDWR);
	if(fd<0)
	{
		perror("open");
		return 0;
	}
	char ch;
	int result;
	write(fd,arr,sizeof(arr));
	lseek(fd,0,SEEK_SET);
	read(fd,temp,sizeof(temp));
	for(int i=0;i<5;i++)
	printf("%d ",temp[i]);
	printf("\n");
}

