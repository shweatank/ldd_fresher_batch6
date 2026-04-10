#include <stdio.h>

void calculateTimes(int n, int bt[], int tq) {
    int wt[n], tat[n], rem_bt[n];
    int time = 0;
    
    // Initialize remaining burst times
    for (int i = 0; i < n; i++) {
        rem_bt[i] = bt[i];
    }

    while (1) {
        int done = 1;
        for (int i = 0; i < n; i++) {
            if (rem_bt[i] > 0) {
                done = 0; // There are still pending processes
                if (rem_bt[i] > tq) {
                    time += tq;
                    rem_bt[i] -= tq;
                } else {
                    time += rem_bt[i];
                    wt[i] = time - bt[i]; // Waiting time = Completion - Burst
                    rem_bt[i] = 0;
                }
            }
        }
        if (done) break;
    }

    // Calculate Turnaround Time (TAT = Burst + Waiting)
    float total_wt = 0, total_tat = 0;
    printf("\nProcess\tBT\tWT\tTAT\n");
    for (int i = 0; i < n; i++) {
        tat[i] = bt[i] + wt[i];
        total_wt += wt[i];
        total_tat += tat[i];
        printf("P%d\t%d\t%d\t%d\n", i + 1, bt[i], wt[i], tat[i]);
    }

    printf("\nAverage Waiting Time: %.2f", total_wt / n);
    printf("\nAverage Turnaround Time: %.2f\n", total_tat / n);
}

int main() {
    int n = 4;
    int burst_times[] = {10, 5, 8, 12}; // Example burst times
    int time_quantum = 2;

    calculateTimes(n, burst_times, time_quantum);
    return 0;
}
/*
1. Track Remaining Burst Times
Create an array rem_bt to store how much execution time each process still needs. Initially, this is equal to the original Burst Time (BT). 
2. Implement Time Slicing
Use a while loop to cycle through all processes. In each turn, if a process has remaining time: 

    If rem_bt > time_quantum, execute it for the full quantum and subtract that from its remaining time.
    If rem_bt <= time_quantum, execute it until it finishes, then record its Completion Time. 

3. Calculate Waiting Time (WT)
The Waiting Time is the total time a process spent in the ready queue. For each process, it can be calculated as:
(This formula assumes all processes arrive at time 0). 
4. Calculate Turnaround Time (TAT)
The Turnaround Time is the total time from arrival to completion. It is calculated as:
or
. 
5. Compute Averages
Sum the individual WT and TAT values for all 4 processes and divide by 4 to find the Average Waiting Time and Average Turnaround Time. 
Summary of Results
The program outputs a table showing the Burst Time, Waiting Time, and Turnaround Time for each process, followed by the calculated system averages*/
