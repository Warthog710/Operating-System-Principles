#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>

//Global variables
int sleepval = 10;
int numReads = 0;
int numWrites = 0;
int readCount = 0;

//Function prototypes
void *reader();
void *writer();

//Volatile boolean to tell a thread to exit cleanly
volatile bool executing = true;

//Mutex locks
pthread_mutex_t mutex;
pthread_mutex_t reading;
pthread_mutex_t wrt;

//Global time program started
unsigned long long msSinceEpoch;

int main(int argc, char* argv[])
{
    int numReaders, numWriters, total, index;

    //Make sure enough parameters are passed
    if (argc < 3)
    {
        //Invalid
        fprintf(stderr, "Usage rwp <#readers> <#writers>\n");
        exit(1);
    }

    //Set readers and writers
    numReaders = atoi(argv[1]);
    numWriters = atoi(argv[2]);
    total = numReaders + numWriters;

    //Seed rand()
    srand(time(0));

    //Intilize locks
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&reading, NULL);
    pthread_mutex_init(&wrt, NULL);

    //Inform user of what was read
    printf("Creating %d reader(s) and %d writer(s)\n", numReaders, numWriters);

    //Dyanmically allocate an array of thread ID's to hold all the information
    pthread_t* threadIds = (pthread_t*)calloc(total, sizeof(pthread_t));

    //Set start time
    struct timeval tv;
    gettimeofday(&tv, NULL);
    msSinceEpoch = (unsigned long long)(tv.tv_sec) * 1000 + (unsigned long long)(tv.tv_usec) / 1000;

    //Create all readers
    for (index = 0; index < numReaders; index++)
    {
        int result = pthread_create(&threadIds[index], NULL, reader, NULL);
    }

    //Create all the writers
    for (index; index < total; index++)
    {
        pthread_create(&threadIds[index], NULL, writer, NULL);
    }

    //Sleep for a given amount of time if some threads were created
    if (total > 0)
    {
        sleep(30);
    }

    //Tell the executing threads to stop executing...
    //? This variable is volatile
    executing = false;

    //Join all the threads
    for (index = 0; index < total; index++)
    {
        pthread_join(threadIds[index], NULL);
    }

    printf("%d threads joined\n", total);

    //Print data points
    printf("\n=====================\n");
    printf("Number of Reads: %d\n", numReads);
    printf("Number of Writes: %d\n", numWrites);

    //Free the allocated memory
    free(threadIds);
}

//Reader thread
void *reader()
{
    while (executing)
    {
        //Get mutual exclusion for readcount
        //! START CS
        pthread_mutex_lock(&mutex);

        //If no one is reading... get the writer lock
        if (readCount == 0)
        {
            pthread_mutex_lock(&wrt);
        }

        //Increment readcount
        readCount++;

        //Unlock mutex
        //! END CS
        pthread_mutex_unlock(&mutex);

        //Get current time
        struct timeval tv;
        gettimeofday(&tv, NULL);
        unsigned long long timeNow = (unsigned long long)(tv.tv_sec) * 1000 + (unsigned long long)(tv.tv_usec) / 1000;

        //Print read
        printf("Reader %lu reads %d at %llums\n", pthread_self(), sleepval, timeNow - msSinceEpoch);

        //Sleep for sleepval ms
        usleep(sleepval * 1000);

        //Get mutual exclusion to update readcount and numReads
        //! START CS
        pthread_mutex_lock(&mutex);
        readCount--;
        numReads++;

        //If last reader, unlock wrt
        if (readCount == 0)
        {
            pthread_mutex_unlock(&wrt);
        }

        //Release
        //! END CS
        pthread_mutex_unlock(&mutex);

        //Sleep for a random number of ms between 100-500
        int sleepTime = (rand() % (500 - 100 + 1)) + 100;
        usleep(sleepTime * 1000);
    }
}

//Writer process
void *writer()
{
    while (executing)
    {
        //Generate random number between 100-1000 and sleep
        int sleepTime = (rand() % (1000 - 100 + 1)) + 100;
        usleep(sleepTime * 1000);

        //After sleeping check executing
        if (!executing)
        {
            break;
        }

        //Generate random number between 10-50
        int updateVal = (rand() % (50 - 10 + 1)) + 10;

        //Gain mutual exclusion to read
        //! START CS
        pthread_mutex_lock(&wrt);

        //Get current time
        struct timeval tv;
        gettimeofday(&tv, NULL);
        unsigned long long timeNow = (unsigned long long)(tv.tv_sec) * 1000 + (unsigned long long)(tv.tv_usec) / 1000;

        //Update sleepVal
        sleepval = updateVal;
        printf("Writer %lu writes %d at %llums\n", pthread_self(), sleepval, timeNow - msSinceEpoch);
        numWrites++;

        //Sleep for sleepval number of ms
        usleep(sleepval * 1000);

        //Release the lock
        //! END CS
        pthread_mutex_unlock(&wrt);
    }
}