#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#define IOCTL_MAGIC 'G'
#define SET_IOC_DATA _IOW(IOCTL_MAGIC , 1 , struct config)
 #define GET_IOC_DATA _IOR(IOCTL_MAGIC , 2 , struct config)
 #define RESET_IOC_DATA _IOR(IOCTL_MAGIC,3,struct config)


struct config {
    int mode;
    int speed;
    char name[32];
};

int main()
{
    int fd = open("/dev/manage_device_config", O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    struct config temp;
    int value;

    printf("Enter the mode and speed");
    scanf("%d%d", &temp.mode,&temp.speed);   // space important

    printf("Enter the device name: ");
    scanf("%s",temp.name);

    
            ioctl(fd, SET_IOC_DATA, &temp);
           
        
            ioctl(fd, GET_IOC_DATA, &temp);
            
    	    printf("Geting the data device\n");
	   
	    printf("Mode = %d\nSpeed = %d\nDevice Name: %s\n",temp.mode,temp.speed,temp.name);
	    	
	    
        
            ioctl(fd, RESET_IOC_DATA, &temp);
            
		printf("Reseting the data device\n");
  
              printf("Mode = %d\nSpeed = %d\nDevice Name: %s\n",temp.mode,temp.speed,temp.name);

        
 
    close(fd);
    return 0;
}
