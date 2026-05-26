#include<stdio.h>
#include<string.h>
#include<stdlib.h>
char * sum(char *str,char *str1)
{
	int len1,len2;
	len1=strlen(str);
	len2=strlen(str1);
	int max=(len1>len2?len1:len2);
	char *res=(char*) malloc(max+2);
	int carry=0;
	int i=len1-1,j=len2-1,k=max-1;
	res[k--]='\0';
	while(i>=0 || j>=0 || carry)
	{
		int sum=carry;
		if(i>=0)	
			sum+=str[i--]-'0';
		if(j>=0)
			sum+=str1[j--]-'0';
		res[k--]=(sum%2)+'0';
		carry=sum/2;
	}
	return res+k+1;
}
int main()
{
	char ch[10],ch1[10];
	printf("Enter 2 strings\n");
	scanf("%s%s",ch,ch1);
	char * ptr=sum(ch,ch1);
	printf("%s\n",ptr);

}
