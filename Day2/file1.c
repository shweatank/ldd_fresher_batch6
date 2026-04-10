#include<stdio.h>
int main(){
 FILE *fp=fopen("r.txt","r");
 char ch;
 do{
    printf("Enter A input : \n");
 }while();

 while((ch=fgetc(fp))!=EOF){
    printf("%c ",ch);
 }
 fclose(fp);
}
