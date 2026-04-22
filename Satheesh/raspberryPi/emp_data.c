#include<stdio.h>
#include<string.h>
#include<fcntl.h>
typedef struct st
{
	int emp_id;
	char emp_name[20];
	char emp_ph[11];
	char emp_pos[20];
	struct st*next;
}node;
node *head=0,*tail=0;
void show()
{
	if(!head)
	{
		printf("No entrys are present \n");
		return;
	}
	node *temp=head;
	while(temp)
	{
		printf("id: %d\n",temp->emp_id);
		printf("Name: %s\n",temp->emp_name);
		printf("phone: %s\n",temp->emp_ph);
		printf("position: %s\n",temp->emp_pos);
		temp=temp->next;
	}
	printf("***************************************\n");
}
void create_node()
{

	node *temp=(node*)malloc(sizeof(node));
	printf("Enter id name phone and position\n");
	scanf("%d%s%s%s",&ptr->emp_id,ptr->emp_name,ptr->emp_ph,ptr->emp_pos);
	if(!head)
	{
		head=temp;
		tail=head;
	}
	else
	{
		tail->next=temp;
		tail=temp;
	}

}
int main()
{
	int op;
	while(1)
	{
		printf("Enter an option\n1)add emp\n2)del emp\n3)show list\n4)exit\n");
		scanf("%d",&op);
		switch (op)
		{
			case 1:
				create_node();
				break;
			case 2:
				del_emp();
				break;
			case 3:
				show();

		}
	}
}
