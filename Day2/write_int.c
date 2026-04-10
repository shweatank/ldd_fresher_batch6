#include <unistd.h> 
#include <stdio.h>  
#include <string.h> 

int main() {
    int num = 5;
    char buf[20]; 
    
    int len = sprintf(buf, "%d\n", num);
    
    write(1, buf, len);
    
}

