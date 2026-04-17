#include<stdio.h>
#include<stdlib.h>
typedef struct node
{
	int data;
	struct node *link;
}NODE;
NODE* input(NODE *head)
{
	NODE *new_node=calloc(1,sizeof(NODE));
	printf("Enter the data\n");
	scanf("%d",&new_node->data);
	if(head==NULL)
	{
		 new_node->link=new_node;
		 return new_node;
	}
	else
	{
		NODE *temp=head;
		while( temp->link!=head)
		{
			temp=temp->link;
		}
		temp->link=new_node;
		new_node->link=head;
	}
	return head;
}
void print(NODE *head)
{
	NODE *temp=head;
	do
	{
		printf("%d ",temp->data);
		temp=temp->link;
	}while(temp!=head);
}
NODE *rev(NODE *head)
{
	NODE *cur=head,*prev=NULL,*next;
	do
	{
		next=cur->link;
		cur->link=prev;
		prev=cur;
		cur=next;

	}while(cur!=head);

	head->link=prev;
	head=prev;
	//last->link=head;
	return head;
}
int main()
{
	NODE *head=NULL;
	int n=5;
	while(n)
	{
		head=input(head);
		n--;
		
	}
	print(head);
	printf("\n");
	head=rev(head);
	print(head);
}
