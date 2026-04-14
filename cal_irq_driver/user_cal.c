#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

enum cal_op {ADD=0,SUB,MUL,DIV};

struct calculator {
   int a;
   int b;
   enum cal_op op;
};

int main(int argc, char *argv[])
{

	if(argc  < 4)
	{
		printf("Usage executable Num1 Num2 Operation (eg : ./a.out 2 3 ADD)");
	     return 0;
	}

	struct calculator cal;

	cal.a = atoi(argv[1]);
	cal.b = atoi(argv[2]);
	if( strcmp( argv[3], "ADD") == 0){
		cal.op = 0;
		printf("operation is ADD\n");
	}
	else if(strcmp(argv[3],"SUB") == 0){
		printf("operation is SUB\n");
		cal.op = 1;
	}
	else if(strcmp(argv[3],"MUL") == 0){
		printf("operation is MUL\n");
		cal.op = 2;
	}
	else if(strcmp(argv[3],"DIV") == 0){
		printf("operation is DIV\n");
		cal.op = 3;
	}
	else{
	     printf("Invalid operation\n Enter correct operation ADD SUB MUL DIV\n");
	}

	//open for writing

	int cal_fd = open("/dev/cal_char",O_RDWR);
	write(cal_fd,&cal,sizeof(cal));
	close(cal_fd);
	long long int result; 
	
	
	//open for reading
	
	cal_fd = open("/dev/cal_char",O_RDONLY);
	int n = read(cal_fd,&result,sizeof(result));
	if(n > 0){
		 printf("Result %lld ",result);
	        return 0;
	}
        close(cal_fd);	 
        return 0;
}
