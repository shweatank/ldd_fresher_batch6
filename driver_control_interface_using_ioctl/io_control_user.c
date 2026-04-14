#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include "ioctl_header.h"

char buffer[256];

static void input_str()
{
        printf("Enter the name of Driver : ");
        fgets(buffer, 32, stdin);
        if( buffer[ strlen( buffer ) - 1 ] == '\n')
              buffer[ strlen( buffer ) -1 ] = '\0';
}

static void display()
{
        printf("1.Set Mode of the device\n");
        printf("2.Get Mode of the device\n");
        printf("3.Clear Buffer\n");
	printf("4.Get write count\n");
	printf("5.Write to buffer\n");
	printf("6.Read buffer\n");
	printf("7.Exit\n");
}

int main()
{

        int fd;
        int choice;
        fd = open("/dev/drv_ioctl_cntrl_interface", O_RDWR);
        if(fd < 0)
        {
                perror("open");
                return 1;
        }

        while(1){
                display();
                scanf("%d",&choice);
                if(choice == 1){
                     int mode;
		     printf("Enter the mode to set (modes : 1-5): ");
		     scanf("%d",&mode);
                     ioctl(fd, SET_MODE, &mode );
		     printf("Mode is set to - %d",mode);
                }
		else if(choice == 2)
                {
			int mode;
                        ioctl(fd, GET_MODE, &mode);
                        printf("Mode is %d\n", mode);
                }
		else if(choice == 3)
                {
                        ioctl(fd, CLEAR_BUFFER);
			printf("Buffer is cleared\n");
                }
		else if(choice == 4)
		{
			int count;
			ioctl(fd ,GET_WRITE_COUNT, &count);
			printf("write count is %d\n",count);
		}
		else if(choice == 5)
		{
			input_str();
			write(fd, buffer, strlen(buffer));
		}
		else if(choice == 6)
		{
			char buffer[256];
			int n = read(fd, buffer, sizeof(buffer));
			buffer[n-1] = '\0';
			printf("Read : %s",buffer);
		}
		else if(choice == 7)
		{
			printf("Terminating program");
			break;
		}

        }
        close(fd);
        return 0;
}

