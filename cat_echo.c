#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#define DEVICE "/dev/calculation_driver"
char write_buff[100];
char read_buff[100];

static int calculator(FILE* fptr)
{
	int num1,num2;

	char opp[5];

	fscanf("%d %d %4s",&num1,&num2,&opp); 
	
	fwrite(&read_buff,sizeof(int),1,fptr);

}
int main()
{
	FILE* fptr=fopen(DEVICE,"rw");
	printf("Enter Numbers and Operation :");
	fd=open(DEVICE,O_RDWR);
	if(fd==-1)
	{
		perror("File is not opened\n");
		return 0;
	}
	printf("Writing into buffer !\n");
	fwrite(&write_buff,sizeof(write_buff),1,fptr);
	
	calculator(fd);
	lseek(fd,0,SEEK_SET);

	int n=read(&read_buff,sizeof(int),1,fptr);;
	if(n>0)
	{
		read_buff[n]='\0';
		printf(" %s\n",read_buff);
	}

	fclose(fptr);

}



