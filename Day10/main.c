#include<stdio.h>
#include "main.h"
int num1,num2,res=0;
char opr;
int main()
{
  printf("Enter two numbers: \n");
  scanf("%d %d",&num1,&num2);
  printf("Enter operator: \n");
  scanf(" %c",&opr);
  switch(opr)
  {
   case '+': res= add(num1,num2);
	     break;
   case '-':res= sub(num1,num2);
	     break;
   case '*': res=mul(num1,num2);
	     break;
   case '/': res=div(num1,num2);
	     break;
   default:
	     printf("invalid\n");
	     break;
  }
  printf("The result: %d\n",res);
  return 0;
}

