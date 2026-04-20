#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<fcntl.h>
int main()
{
        int fd;
        char str[256];
        char ptr[256];
        fd=open("/dev/15_1",O_RDWR);
        if(fd==-1)
        {
                perror("open");
                return 0;
        }

        fgets(str,100,stdin);
        if(str[strlen(str)-1]=='\n')
        {
                str[strlen(str)-1]='\0';
        }

        write(fd,str,strlen(str)+1);
        while(1)
        {
             read(fd,ptr,sizeof(ptr));
             printf("received data from kernel=%s\n",ptr);
             if(ptr[2]=='5')
             {
                     break;
             }
        }
        return 0;

}
