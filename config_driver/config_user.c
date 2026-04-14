#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>


struct config_t {
    int mode;
    int speed;
    char name[32];
};

static struct config_t config;

#define CONFIG_IOCTL_MAGIC 'A'
#define SET_CONFIG_ _IOWR(CONFIG_IOCTL_MAGIC, 1, struct config_t)
#define GET_CONFIG_ _IOWR(CONFIG_IOCTL_MAGIC, 2, struct config_t)
#define RESET_DEFAULT_CONFIG_ _IOWR(CONFIG_IOCTL_MAGIC, 3, struct config_t)

static void input_settings()
{
	printf("Enter the name of Driver : ");
	fgets(config.name, 32, stdin);
	if( config.name[ strlen( config.name ) - 1 ] == '\n')
	      config.name[ strlen( config.name ) -1 ] = '\0';

	printf("Enter the config settings for the driver\nEnter mode of driver : ");
	scanf("%d",&config.mode);
	printf("Enter speed of the driver : ");
	scanf("%d",&config.speed);
	printf("Enter the name of Driver : ");
}

static void display()
{
	printf("1.Set configurations settings\n");
	printf("2.Get configuration settings\n");
	printf("3.Reset configuration settings\n");
}

int main()
{
	
	int fd;
	int choice;
	fd = open("/dev/config_driver", O_RDWR);
	if(fd < 0)
	{
		perror("open");
		return 1;
	}

	while(1){
		display();
		scanf("%d",&choice);
		if(choice == 1){
		     input_settings();
		     ioctl(fd, SET_CONFIG_, &config);
		}
		if(choice == 2)
		{
			ioctl(fd, GET_CONFIG_, &config);
			printf("Mode = %d\nSpeed = %d\nName =%s\n", config.mode, config.speed, config.name);
		}
		if(choice == 3)
		{
			ioctl(fd, RESET_DEFAULT_CONFIG_);
		}
	
	}
	close(fd);
	return 0;
}
