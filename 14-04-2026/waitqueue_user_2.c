#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<fcntl.h>
int main()
{
        char str[256];
        char ptr[256];
        int fd;
        fd=open("/dev/waitqueue_2",O_RDWR);
        if(fd==-1)
        {
                perror("open");
                return 0;
        }
        fgets(ptr,100,stdin);
        if(ptr[strlen(ptr)-1]=='\n')
        {
                ptr[strlen(ptr)-1]='\0';
        }
        write(fd,ptr,strlen(ptr)+1);

        read(fd,str,5);
        str[5]='\0';
        printf("%s\n",str);

        read(fd,ptr,5);
        ptr[5]='\0';
        printf("%s\n",ptr);

        close(fd);
}
