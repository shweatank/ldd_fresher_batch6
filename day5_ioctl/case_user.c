#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<string.h>

int main(){

char read_buff[100];
char write_buff[100];

/* opening file to write */
int fd=open("/dev/basic_char",O_WRONLY);
if(fd<0){
perror("Error in opening file to write!\n");
return 1;
}

printf("enter the string: ");
fgets(write_buff,sizeof(write_buff),stdin); //commandline input

write(fd,write_buff,strlen(write_buff)); //write the string

close(fd);

/*opening file to read */
fd=open("/dev/basic_char",O_RDONLY);
if(fd<0){
perror("Error in opening file to read!\n");
return 1;
}

int bytes=read(fd,read_buff,(sizeof(read_buff)-1));//read from file
if(bytes>0){
read_buff[bytes]='\0';
} 
printf("Output from the driver with changed case: %s\n",read_buff);
close(fd);

}
