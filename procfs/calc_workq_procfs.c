#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#define DEVICE "/proc/proc_name"
int num1,num2;
char opp[4];
int main()
{
	FILE *fp=fopen(DEVICE,"r+");
	printf("Enter the Num1 and Num2 and Operation :");
	scanf("%d %d %s",&num1,&num2,opp);
	if(fp==NULL)
	{
		perror("File is not opened\n");
		return 0;
	}
	printf("Writing into buffer !\n");
	fprintf(fp,"%d %d %s",num1,num2,opp);
	fflush(fp);
	rewind(fp);
	int res;
	int n=fscanf(fp,"%d",&res);
	if(n>0)
	{
		printf("Result is  %d\n",res);
	}

	fclose(fp);

}



