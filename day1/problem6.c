#include<stdio.h>
int main()
{
	int n=3;
	int at[]={0,1,2};
	int bt[]={5,3,1};
	int rt[]={5,3,1};
	int tq=2;
	int wt[3]={0};
	int tat[3]={0};
	int ct[3]={0};
	int time=0,done=1;
	do
	{
		done=1;
		for(int i=0;i<n;i++)
		{
			if(at[i]<=time&&rt[i]>0)
			{
				done=0;
				if(rt[i]>tq)
				{
					time+=tq;
					rt[i]-=tq;
				}
				else
				{
					time+=rt[i];
					ct[i]=time;
					rt[i]=0;
				}
			}
		}
		int flag=1;
		for(int i=0;i<n;i++)
		{
			if(at[i]<=time&&rt[i]>0)
			{
				flag=0;
				break;
			}
		}
		if(flag)
		{
			time++;
			done=1;
		}
	}while(!done);
	for(int i=0;i<n;i++)
	{
		tat[i]=ct[i]-at[i];
		wt[i] =tat[i]-bt[i];
	}
	printf("process\tAT\tBT\tCT\tTAT\tWT\n");
	for(int i=0;i<n;i++)
	{
		printf("P%d\t%d\t%d\t%d\t%d\t%d\n",i+1,at[i],bt[i],ct[i],tat[i],wt[i]);
	}
}
