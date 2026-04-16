#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

enum cal_op {ADD=0,SUB,MUL,DIV};

struct calculator {
   int a;
   int b;
   int result;
};

int main(int argc, char *argv[])
{

	if(argc  < 3)
	{
		printf("Usage executable Num1 Num2 Operation (eg : ./a.out 2 3)");
	     return 0;
	}

	struct calculator cal;

	cal.a = atoi(argv[1]);
	cal.b = atoi(argv[2]);

	//open for writing

	int cal_fd = open("/dev/scancode_cal",O_RDWR);
	write(cal_fd,&cal,sizeof(cal));
	close(cal_fd); 
	
	//open for reading
	
	cal_fd = open("/dev/scancode_cal",O_RDONLY);
	int n = read( cal_fd, &cal, sizeof(cal));
	if(n > 0){
		 printf("Result %d \n",cal.result);
	        return 0;
	}
        close(cal_fd);	 
        return 0;
}
