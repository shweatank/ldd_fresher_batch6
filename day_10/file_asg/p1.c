#include <stdio.h>
#include <fcntl.h>

struct employee{
char name[20];
int id;
char location[20];
struct employee * next;
};

void add_begin(struct employee **);
void print_data(struct employee *);
void delete_node(struct employee **,int);
int count(struct employee *);

FILE *fd;
fd =fopen("data.txt","w");


int main()
{
struct employee * hptr=0;
int op;
char ch;
while(1){
printf("select the option 1) add the data 2)delete the node 3) print the data 4) exit from the code\n");
scanf("%d",&op);
switch(op){
	case 1:
		add_begin(&hptr);
		break;
	case 2:
		delete_node(&hptr);
		break;
	case 3:
		print_data(hptr);
		break;
	case 4:
		printf("program completed\n");
		return 0;
	default:
		printf("enter the correct option\n");
	
}
}

}


void add_begin(struct employee ** ptr)
{
struct employee * temp=(struct employee *)malloc(sizeof(struct employee));

printf("enter the data like name id location\n");
scanf("%s %d %s",temp->name,&temp->id,temp->location);

temp -> next=*ptr;
*ptr=temp;

fprintf(fd,"%s %d %s",temp->name,temp->id,temp->location);
}



int count(struct employee *ptr)
{
int count=0;
while(*ptr->next!=0){
count++;
}
return count;
}

void delete_node(struct employee ** ptr,int id)
{
	struct employee *temp;
	temp=*ptr;
	if(temp->id==id){
	*ptr=temp->next;
	free(temp);
	}
	while(temp->next->id!=id){
	temp=temp->next;
	}

}
