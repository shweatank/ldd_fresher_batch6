#include<stdio.h>
void case_conv(char *str){
  for(int i=0;str[i]!='\0';i++){
    str[i] ^=32;
  }
}
int main(){
 char str[10]="rAHUl"; 
 case_conv(str);
 puts(str);
}
