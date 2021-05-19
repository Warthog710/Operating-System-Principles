#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "process.h"

//Write a generic struct to hold a process
//The struct keeps track of various datapoints, waiting time, response time, & turnaround time
//CPU usage is tracked, if no process is running but more are going to arrive, that is down time.
//Use a circular linked list to implement the process queue

//Function Prototypes
Process* readInputFile(char fileName[]);
Process* sortProcessList(Process* head);
Process* selectNextProcessSRTF(Process* head, int systemTime);
void swap(Process* head, Process* next);
void FCFS(Process* head);
int nextProcess(Process* rdyQueue[], int rear);
int updateReadyQueue(Process* rdyQueue[], Process* head, int rear, int systemTime);
void updateWaitingTime(Process* head, Process* currentProcess, int systemTime);
void SRTF(Process* head);
void RR(Process* head, int timeQuantum);
void printReport(Process* head, int systemTime, int idleTime);

//Global variable
int NUM_PROCESSES = 0;

int main(int argc, char* argv[])
{
    int count;

    //Make sure enough parameters are passed
    if (argc < 3)
    {
        //Invalid parameters passed
        fprintf(stderr, "Usage: sim input_file [FCFS|RR|SRTF] [time_quantum]\n");
        exit(1);
    }

    //Read the input file
    Process* head = readInputFile(argv[1]);

    //Uppercase scheduling algorithm parameter
    char* temp = argv[2];

    for (count = 0; count < strlen(argv[2]); count++)
    {
        *temp = toupper(*temp);
        temp++;
    }

    //Run correct algorithm
    //Run the FCFS algorithm
    if (strcmp(argv[2], "FCFS") == 0)
    {
        printf("Scheduling Algorithm: %s\n", argv[2]);
        printf("%d total tasks read from \"%s\". Press 'enter' to start...\n", NUM_PROCESSES, argv[1]);
        printf("============================================================");

        //Wait for enter to be pressed
        while(getchar() != '\n');

        //Run FCFS
        FCFS(head);
        exit(0);
    }
    //Run the RR algorithm
    else if (strcmp(argv[2], "RR") == 0 && argc > 3)
    {
        //Make sure a number was passed
        if (isdigit(atoi(argv[3])))
        {
            fprintf(stderr, "Error: RR can only accept numbers as a time quantum\n");
        }
        //Verify time quantum passed is greater than zero
        else if (atoi(argv[3]) <= 0)
        {
            fprintf(stderr, "Error: Cannot except a time quantum <= 0\n");
        }
        //Run the RR algorithm
        else
        {
            printf("Scheduling Algorithm: %s (Time Quantum: %d)\n", argv[2], atoi(argv[3]));
            printf("%d total tasks read from \"%s\". Press 'enter' to start...\n", NUM_PROCESSES, argv[1]);
            printf("============================================================");

            //Wait for enter to be pressed
            while(getchar() != '\n');

            //Run RR
            RR(head, atoi(argv[3]));   
            exit(0);     
        }
    }
    //Run the SRTF algorithm
    else if(strcmp(argv[2], "SRTF") == 0)
    {
        printf("Scheduling Algorithm: %s\n", argv[2]);
        printf("%d total tasks read from \"%s\". Press 'enter' to start...\n", NUM_PROCESSES, argv[1]);
        printf("============================================================");

        //Wait for enter to be pressed
        while(getchar() != '\n');

        //Run FCFS
        SRTF(head);
        exit(0);
    }

    //All other cases, assume bad input
    fprintf(stderr, "Usage: sim input_file [FCFS|RR|SRTF] [time_quantum]\n");
    exit(1);
}

//! File Format: pid arrival_time burst_time
//Parses a file with the above format, then calls sort on the list created
Process* readInputFile(char fileName[])
{
    FILE* fp = fopen(fileName, "r");
    Process* head = NULL;
    Process* current = NULL;

    //If the file could not be opened... quit the program.
    if (fp == NULL)
    {
        fprintf(stderr, "%s failed to open... Does it exist?\nExiting Program...\n", fileName);
        exit(1);
    }

    //Iterate through the file, creating a doubly linked list
    while (!feof(fp))
    {
        //Dynamically allocate memory
        Process* temp = (Process*)malloc(sizeof(Process));
        NUM_PROCESSES++;
        
        //If head pointer is not set, make this node the head
        if (head == NULL)
        {
            head = temp;        
        }

        //Read the line into the process structure
        fscanf(fp, "%d %d %d", &temp->pid, &temp->arrivalTime, &temp->burstTime);

        //If burst time == 0, the process is invalid, discard it
        //! Fix for blank lines at the end of the file. Just in case...
        if (temp->burstTime <= 0)
        {
            free(temp);
            NUM_PROCESSES--;
            continue;
        }

        //Set parameters to initial value
        temp->responseTime = -1;
        temp->turnAroundTime = 0;
        temp->waitTime = 0;

        //If current is set, set the pointer of the previous node
        if (current != NULL)
        {
            current->next = temp;
        }

        //Set current
        current = temp;      
    }

    //Close the file
    fclose(fp);

    //Set the last nodes next pointer to NULL and sort.
    current->next = NULL;
    head = sortProcessList(head);

    //Return head
    return head;
}

//Sorts a list and makes it circular
Process* sortProcessList(Process* head)
{
    Process* start = head;
    Process* temp = NULL;
    Process* previous = NULL;
    int count;

    //Make sure the list isn't empy
    if (head == NULL)
    {
        return head;
    }

    //Simple selection sort (Nothing too complex as there is only ever going to be 20 items to sort in the assignment description)
    while (start != NULL)
    {
        //Loop through the list find the min... if their is a conflict keep the one with the lower PID
        temp = start;
        Process* min = temp;

        while (temp != NULL)
        {
            //Select a min
            if (temp->arrivalTime <= min->arrivalTime)
            {
                //If their is a tie... prefer the lower PID
                if (temp->arrivalTime == min->arrivalTime)
                {
                    //printf("tie\n");
                    if (min->pid < temp->pid)
                    {
                        //printf("%d Vs %d\n", temp->pid, min->pid);
                        temp = temp->next;
                        continue;
                    }                    
                }
                min = temp;
            }
            temp = temp->next;
        }
        //printf("MIN: %d %d %d\n", min->pid, min->arrivalTime, min->burstTime);

        swap(start, min);
        previous = start;
        start = start->next;
    }

    //Make the list circular
    previous->next = head;

    return head;
}

//Swaps the data of two pass process nodes
void swap(Process* head, Process* next)
{
    int pid = head->pid;
    int aT = head->arrivalTime;
    int bT = head->burstTime;

    head->pid = next->pid;
    head->arrivalTime = next->arrivalTime;
    head->burstTime = next->burstTime;

    next->pid = pid;
    next->arrivalTime = aT;
    next->burstTime = bT;
}

//Simulates the First Come First Serve (FCFS) CPU scheduling algorithm
void FCFS(Process* head)
{
    int processCompleted = 0;
    int systemTime = 0;
    int timeNotExecuting = 0;

    while (1)
    {
        //If the process has arrived, execute
        if (head->arrivalTime <= systemTime)
        {
            //If the response time has yet to be set, set it.
            if (head->responseTime < 0)
            {
                head->responseTime = systemTime - head->arrivalTime;
            }

            //If burst time is zero, move onto the next process
            if (head->burstTime <= 0)
            {
                //Set old process datapoint
                head->turnAroundTime = systemTime - head->arrivalTime;

                printf("<system time %4d> ", systemTime);
                printf(" process %4d has finished......\n", head->pid);

                head = head->next;

                processCompleted++;

                //If all the process have been completed
                if (processCompleted >= NUM_PROCESSES)
                {
                    printf("<system time %4d> ", systemTime);
                    printf(" All processes finished.........\n");
                    break;
                } 

                //Set new process datapoint
                head->waitTime = systemTime - head->arrivalTime;

                continue;               
            }

            //Print process info
            printf("<system time %4d> ", systemTime);
            printf(" process %4d is running\n", head->pid);

            //Decrement burst time and increment system time
            head->burstTime--;
            systemTime++;
        }
        //The system must be idle...
        else
        {
            printf("<system time %4d> ", systemTime);
            printf(" system is idle......\n");
            timeNotExecuting++;
            systemTime++;
        }        
    }

    //Print a report
    printReport(head, systemTime, timeNotExecuting);
}

void RR(Process* head, int timeQuantum)
{
    Process* rdyQueue[NUM_PROCESSES];
    int processesCompleted = 0;
    int systemTime = 0;
    int timeNotExecuting = 0;
    int front = NUM_PROCESSES - 1;
    int rear = NUM_PROCESSES - 1;
    int count;

    //Intilize ready queue
    rear = updateReadyQueue(rdyQueue, head, rear, systemTime); 

    while (1)
    {
        //If all process have completed... 
        if (processesCompleted >= NUM_PROCESSES)
        {
            printf("<system time %4d> ", systemTime);
            printf(" All processes finished.........\n");
            break;                
        }

        //If the ready queue is empty... the system must be idle
        if (front == rear)
        {
            printf("<system time %4d> ", systemTime);
            printf(" system is idle......\n");
            timeNotExecuting++;
            systemTime++;

            //Check for new processes
            rear = updateReadyQueue(rdyQueue, head, rear, systemTime); 
            continue;
        }

        //Run the process at the front of the ready queue
        for (count = 0; count < timeQuantum; count++)
        {   
            //If response time has yet to be set, set it.
            if (rdyQueue[front]->responseTime < 0)
            {
                rdyQueue[front]->responseTime = systemTime - rdyQueue[front]->arrivalTime;
            }

            //Run the process
            printf("<system time %4d> ", systemTime);
            printf(" process %4d is running\n", rdyQueue[front]->pid);

            //Update wait time of all processes
            updateWaitingTime(head, rdyQueue[front], systemTime);

            //Decrement burst time
            rdyQueue[front]->burstTime--;
            systemTime++;

            //Check for new processes
            rear = updateReadyQueue(rdyQueue, head, rear, systemTime); 

            //If burst time is zero, get a new process
            if(rdyQueue[front]->burstTime <= 0)
            {
                //Set datapoint
                rdyQueue[front]->turnAroundTime = systemTime - rdyQueue[front]->arrivalTime;

                processesCompleted++;
                printf("<system time %4d> ", systemTime);
                printf(" process %4d has finished......\n", rdyQueue[front]->pid);
                break;
            }     
        }

        //Get the next process
        rear = nextProcess(rdyQueue, rear);             
    }

    //Print report
    printReport(head, systemTime, timeNotExecuting);
}

//Moves the current front to the rear of a ready queue, or removes it...
int nextProcess(Process* rdyQueue[], int rear)
{
    int location = NUM_PROCESSES - 1;
    int index;
    Process* temp;

    //Grab the front process
    Process* front = rdyQueue[NUM_PROCESSES - 1];

    //Move all elements up one
    for (index = (NUM_PROCESSES - 1); index > (rear + 1); index--)
    {
        rdyQueue[index] = rdyQueue[index - 1];
    }

    //If the grabbed process still has burst time add it to the rear
    if (front->burstTime > 0)
    {
        rdyQueue[rear + 1] = front;
    }
    //Discard the process
    else
    {
        return rear += 1;
    } 

    return rear;
}

//Builds and updates a ready queue
int updateReadyQueue(Process* rdyQueue[], Process* head, int rear, int systemTime)
{
    int dupeFlag = 0;
    int count, index;

    for (count = 0; count < NUM_PROCESSES; count++)
    {
        //If the system time equals arrive time... add the process
        if(head->arrivalTime == systemTime)
        {
            rdyQueue[rear] = head;
            rear--;
        }
        head = head->next;
    }

    return rear;    
}

//Simulates the Shortest Remaining Time First (SRTF) scheduling algorithm
void SRTF(Process* head)
{
    Process* currentProcess = NULL;
    int processCompleted = 0;
    int systemTime = 0;
    int timeNotExecuting = 0;

    //Select a process to run...
    while(1)
    {
        //Select next process
        currentProcess = selectNextProcessSRTF(head, systemTime);

        //No process can be ran... System must be idle
        if (currentProcess == NULL)
        {
            printf("<system time %4d> ", systemTime);
            printf(" system is idle......\n");
            timeNotExecuting++;
            systemTime++;
            continue;
        }

        //If the response time has yet to be set, set it.
        if (currentProcess->responseTime < 0)
        {
            currentProcess->responseTime = systemTime - currentProcess->arrivalTime;
        }

        //If burst time is zero, get a new process
        if (currentProcess->burstTime <= 0)
        {
            //Set process datapoint
            currentProcess->burstTime = -1;
            currentProcess->turnAroundTime = systemTime - currentProcess->arrivalTime;

            printf("<system time %4d> ", systemTime);
            printf(" process %4d has finished......\n", currentProcess->pid);

            processCompleted++;

            //Check to see if all processes have completed
            if (processCompleted >= NUM_PROCESSES)
            {                    
                printf("<system time %4d> ", systemTime);
                printf(" All processes finished.........\n");
                break;
            }

            continue;
        }

        //Run the process
        printf("<system time %4d> ", systemTime);
        printf(" process %4d is running\n", currentProcess->pid);

        //Update waiting time
        updateWaitingTime(head, currentProcess, systemTime);

        //Decrement burst time
        currentProcess->burstTime--;
        systemTime++;
    }

    //Print a report
    printReport(head, systemTime, timeNotExecuting);
}

//Select the next process to run for the SRTF scheduling algorithm
Process* selectNextProcessSRTF(Process* head, int systemTime)
{
    Process* returnProcess = NULL;
    int count;
    int minBurst = -1;

    //Loop through the processes, and return the one with the lowest burst time
    for (count = 0; count < NUM_PROCESSES; count++)
    {
        if ((head->burstTime < minBurst || minBurst == -1) && head->arrivalTime <= systemTime && head->burstTime != -1)
        {
            minBurst = head->burstTime;
            returnProcess = head;
        }
        head = head->next;
    }

    //Note, returns NULL, if no process can be selected.
    //Note, a process with a burst time of zero can be returned. This is intentional and handled in SRTF().
    return returnProcess;
}

//Updates the waiting time of all active processes that are not currently executing
void updateWaitingTime(Process* head, Process* currentProcess, int systemTime)
{
    int count;

    //Iterate through all process, if they've arrived and are not running increment waiting time
    for (count = 0; count < NUM_PROCESSES; count++)
    {
        if (head != currentProcess && head->arrivalTime <= systemTime && head->burstTime > 0)
        {
            //Increment the process wait time
            head->waitTime++;
        }

        head = head->next;
    }   
}

//Prints averages, and free's the allocated memory
void printReport(Process* head, int systemTime, int idleTime)
{
    int avgWaitingTime = 0;
    int avgResponseTime = 0;
    int avgTurnaroundTime = 0;
    int index;

    //Loop through all the processes and collect the datapoints
    for (index = 0; index < NUM_PROCESSES; index++)
    {
        /*printf("Process %d\n", head->pid);
        printf("Waiting Time %d ", head->waitTime);
        printf("Response Time %d ", head->responseTime);
        printf("Turnaround Time %d\n\n", head->turnAroundTime);*/

        avgWaitingTime += head->waitTime;
        avgResponseTime += head->responseTime;
        avgTurnaroundTime += head->turnAroundTime;

        Process* temp = head;
        head = head->next;  

        //Free the memory
        free(temp);   
    }

    //Print averages
    printf("\n============================================================\n");
    printf("Average CPU Usage: %.2f%%\n", 100 - (100 * ((float)idleTime / systemTime)));
    printf("Average Waiting Time: %.2f\n", (float)avgWaitingTime / NUM_PROCESSES);
    printf("Average Response Time: %.2f\n", (float)avgResponseTime / NUM_PROCESSES);
    printf("Average Turnaround Time: %.2f\n", (float)avgTurnaroundTime / NUM_PROCESSES);
    printf("============================================================\n");
}