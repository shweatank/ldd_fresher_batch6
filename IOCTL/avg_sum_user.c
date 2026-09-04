#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>

#define IOCTL_MAGIC 'G'

#define SET_IOC_DATA _IOW(IOCTL_MAGIC , 1 , struct data)
 #define GET_IOC_DATA _IOR(IOCTL_MAGIC , 2 , struct data)

struct data {
    int size;
    int in_arr[20];
    int op_arr[2];
};

int main()
{
    int fd = open("/dev/avg_sum_arr", O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    struct data temp;
 
    printf("Enter the size of array:  ");
    scanf("%d", &temp.size);   // space important

    printf("Enter the array elements: ");
for(int i = 0; i < temp.size;i++)
{
    scanf("%d", &temp.in_arr[i]);
}
    
            ioctl(fd, SET_IOC_DATA, &temp);
          
        
            ioctl(fd, GET_IOC_DATA,&temp);
            
	     close(fd);
           
    

    
    

    printf("Result from kernel sum : %d avg : %d\n", temp.op_arr[0],temp.op_arr[1]);

    close(fd);
    return 0;
}
