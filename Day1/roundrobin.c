#include<stdio.h>
#include<stdlib.h>

void calculateRR(int n,int bt[],int quantum)
{
  int wt[n],tat[n],rem_bt[n],cur_t=0,done;
  for(int i=0;i<n;i++) rem_bt[i]=bt[i];

  do
  {
     done=1;
    for(int i=0;i<n;i++){
	 if(rem_bt[i]>0)
	 {
	   done=0;
	   if(rem_bt[i]>quantum)
	   {
	     cur_t+=quantum;
	     rem_bt[i]-=quantum;
	   }
	   else
	   {
		cur_t+=rem_bt[i];
		wt[i]=cur_t-bt[i];
		rem_bt[i]=0;
	   }
        }
	    
    }
 }while(!done);

   for (int i = 0; i < n; i++) {
        tat[i] = bt[i] + wt[i];
    }

   printf("\nProcess\tBT\tWT\tTAT\n");
    for (int i = 0; i < n; i++)
    {
        printf("P%d\t%d\t%d\t%d\n", i + 1, bt[i], wt[i],tat[i]);
    }
}
int main()
{
  int n=4,quantum=2;
  int burst_time[]={10,5,8,6};
   calculateRR(n,burst_time,quantum);
   return 0;
}
