#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
int main()
{
        int fd;
        char wbuf[100];
        char rbuf[100];
        fd=open("/proc/proc_demo",O_RDWR);
	if(fd<0)
	{
		perror("open");
		return -1;
	}
        printf("enter the data:");
        fgets(wbuf,sizeof(wbuf),stdin);
        write(fd,wbuf,sizeof(wbuf));
        printf("Waiting for the data..\n");
        read(fd,rbuf,sizeof(rbuf));
        printf("Received :%s\n",rbuf);
        close(fd);
        return 0;
}

