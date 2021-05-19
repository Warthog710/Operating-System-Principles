typedef struct Process 
{
    int pid;
    int arrivalTime;
    int burstTime;

    int waitTime;
    int turnAroundTime;
    int responseTime;

    //Process pointer for linked list
    struct Process* next;

} Process;