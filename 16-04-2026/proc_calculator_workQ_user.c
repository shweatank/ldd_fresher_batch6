#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<fcntl.h>
int main()
{
        char str[126];
        char ptr[126];
        int fd=open("/proc/proc_calculator_workQ",O_RDWR);
        if(fd==-1)
        {
                perror("open");
                return 0;
        }
        fgets(str,sizeof(str),stdin);
        if(str[strlen(str)-1]=='\n')
        {
                str[strlen(str)-1]='\0';
        }

        write(fd,str,strlen(str));
     read(fd,ptr,sizeof(ptr));
        printf("Received value=%s\n",ptr);
        close(fd);
}
