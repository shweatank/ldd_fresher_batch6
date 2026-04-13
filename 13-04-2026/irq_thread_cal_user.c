#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<string.h>
int main()
{
        int fd;
        char buffer[256];
        fd=open("/dev/irq_thread_cal",O_RDWR);
        if(fd==-1)
        {
                perror("open");
        }
        fgets(buffer,256,stdin);
        if(buffer[strlen(buffer)-1]=='\n')
        {
                buffer[strlen(buffer)-1]='\0';
        }
        write(fd,buffer,strlen(buffer));
        close(fd);
}
