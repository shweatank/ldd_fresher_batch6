#include<stdio.h>
typedef struct employee
{
	char name[20];
	int age;
	char address[30];
	struct employee* link;
}Slist;
Slist* delete_node_at_given_position(Slist *head,int pos)
{
        if(pos<0&&head==NULL)
        {
                printf("Enter valid position.\n");
                return head;
        }
        if(pos==0)
        {
                Slist* temp=head;
                head=head->link;
                free(temp);
                return head;
        }
        Slist* temp=head;
        int i=0;
        Slist* prev=NULL;
        while(i<pos&&temp!=NULL)
        {
                prev=temp;
                temp=temp->link;
                i++;
        }
        if(temp==NULL)
        {
                printf("Enter valid Position.\n");
                return head;
        }
        prev->link=temp->link;
        free(temp);
        return head;
}
Slist* add_employee(Slist* head)
{
        Slist* new=create_node();
	printf("Enter the details of employee\n");
	scanf("%s"
        new->value=;
        if(head==NULL)
        {
                head=new;
                return head;
        }
        Slist* temp=head;
        while(temp->link!=NULL)
        {
                temp=temp->link;
        }
        temp->link=new;
        return head;
}
void print_employee(Slist* head)
{
        Slist* temp=head;
        if(temp==NULL)
        {
                printf("The Linked List is Empty.\n");
                return ;
        }
        while(temp!=NULL)
        {
                printf("%s %d %s",temp->name,temp->age,temp->address);
                printf("\n");
                temp=temp->link;
        }
        printf("NULL\n");
}

int main()
{
	Slist* head;
	int sel=0;
	while(1)
	{
		printf("Enter the operation to be performed\n1.Add details\n2.Delete details\n");
		scanf("%d",&sel):
		switch(sel)
		{
			case 1:
				head=add_employee(head);
				print_employee(head);
				break;
			case 2:
				head=delete_employee(head);
				print_employee(head);
				break;
			default:
				printf("Enter correct Option\n");
		}

	}

	
}
	
