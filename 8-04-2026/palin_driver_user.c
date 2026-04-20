#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<fcntl.h>
int main()
{
        int fd=open("/dev/palin_driver",O_WRONLY);
        if(fd==-1)
        {
                perror("open");
                return 0;
        }
        char str[100];
        fgets(str,100,stdin);
       if(str[strlen(str)-1]=='\n')
       {
               str[strlen(str)-1]='\0';
       }

       printf("string=%s\n",str);

       if(write(fd,str,strlen(str))<=0)
       {
               perror("write");
               return 0;
       }
       close(fd);

       int fdd=open("/dev/palin_driver",O_RDONLY);
          if(fdd==-1)
          {
                  perror("open");
                  return 0;
          }
          char ptr[100];
 
         if(read(fdd,ptr,sizeof(ptr))<=0)
         {
                 perror("read");
                 return 0;
         }
         printf("final value=%s\n",ptr);
         close(fdd);
         return 0;
}

