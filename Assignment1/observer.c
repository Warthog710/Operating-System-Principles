#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/param.h>
#include <time.h>

//Funtion prototypes
void shortReport();
void mediumReport();
void longReport(int interval, int duration);
void printSearchTerm(FILE *openFile, char searchTerm[]);
void printUpTime();
void printUSICPUTime();
void printBootDate();
void printNumberReadsWrites();
void printMemoryInfo();
float printSampledLoadAvg();

//Main function to execute
int main(int argc, char *argv[])
{
    struct timeval now;
    char c1, c2;
    char repTypeName[16] = "Standard";
    char lineBuffer[50];
    int interval, duration;

    //If there arguments we need to consider
    if (argc > 1)
    {
        sscanf(argv[1], "%c%c", &c1, &c2);
        if (c1 != '-')
        {
            //Invalid parameters passed
            fprintf(stderr, "usage: observer [-s] [-l int dur]\n");
            exit(1);
        }
        if (c2 == 's')
        {
            strcpy(repTypeName, "Short");
        }
        if (c2 == 'l')
        {
            //Verify number of parameters is correct
            if (argc >= 4)
            {
                strcpy(repTypeName, "Long");
                interval = atoi(argv[2]);
                duration = atoi(argv[3]);
            }
            else
            {
                //Invalid parameters passed
                fprintf(stderr, "usage: observer [-s] [-l int dur]\n");
                exit(1);
            }
        }
    }

    //Report date (gettimeofday) and machine hostname
    gettimeofday(&now, NULL);
    printf("Status report type %s at %s\n", repTypeName, ctime(&(now.tv_sec)));

    FILE *fp = fopen("/proc/sys/kernel/hostname", "r");

    if (fp == NULL)
    {
        fprintf(stderr, "/proc/sys/kernel/hostname failed to open... does it exist?\n\n");
    }
    else
    {
        fgets(lineBuffer, sizeof(lineBuffer), fp);
        printf("Machine Hostname: %s\n", lineBuffer);
        fclose(fp);
    }

    //Call the correct report version
    if (!strcmp(repTypeName, "Long"))
    {
        longReport(interval, duration);
    }
    else if (!strcmp(repTypeName, "Short"))
    {
        mediumReport();
    }
    else
    {
        shortReport();
    }

    //Exit program
    exit(0);
}

//Prints short report
void shortReport()
{
    FILE *fp;
    char lineBuffer[200];
    int time;

    //CPU Model Name
    fp = fopen("/proc/cpuinfo", "r");

    if (fp == NULL)
    {
        fprintf(stderr, "/proc/cpuinfo failed to open... does it exist?\n\n");
    }
    else
    {
        printf("CPU Model Name");
        printSearchTerm(fp, "model name");
        fclose(fp);
    }

    //Kernel Version
    fp = fopen("/proc/version", "r");

    if (fp == NULL)
    {
        fprintf(stderr, "/proc/version failed to open... does it exist?\n\n");
    }
    else
    {
        fgets(lineBuffer, sizeof(lineBuffer), fp);
        printf("Kernel Version: %s\n", lineBuffer);
        fclose(fp);
    }

    //Amount of time since the system last booted in dd:hh:mm:ss form
    printUpTime();
}

//Prints the medium report (first prints short report)
void mediumReport()
{
    FILE *fp;
    char linebuffer[200];

    //Call short report
    shortReport();

    //The amount of time all CPU's have spent in user mode, system mode, and idle
    printUSICPUTime();

    //The number of disk read/write requests completed on the system
    printNumberReadsWrites();

    //The number of context switches that the kernel has performed
    fp = fopen("/proc/stat", "r");

    if (fp == NULL)
    {
        fprintf(stderr, "/proc/stat failed to open... does it exist?\n\n");
    }
    else
    {
        printf("Context Switches Performed: ");
        printSearchTerm(fp, "ctxt");
        fclose(fp);
    }

    //The time when the system was last booted
    printBootDate();

    //The number of processes that have been created since the system was booted
    fp = fopen("/proc/stat", "r");

    if (fp == NULL)
    {
        fprintf(stderr, "/proc/stat failed to open... does it exist?\n\n");
    }
    else
    {
        printf("Processes Created since System Boot: ");
        printSearchTerm(fp, "processes");
        fclose(fp);
    }
}

//Prints the long report (first prints medium report)
void longReport(int interval, int duration)
{
    int iteration = 0;
    float loadAvg = 0;

    //Call medium report
    mediumReport();

    //Print amount of memory configured and currently available on the system
    printMemoryInfo();

    printf("Load average (sampled every %d seconds for %d seconds):\n", interval, duration);

    //Calculate the load average
    while (iteration < duration)
    {
        sleep(interval);
        loadAvg += printSampledLoadAvg();
        iteration += interval;
    }

    //Print overall average
    printf("\nAverage load over sampled duration: %.0f%%\n", (loadAvg * 100) / (duration / interval));
}

//Searches through the file until it finds the search term line and then prints it
//If the search term is not found, it prints "NOT FOUND"
void printSearchTerm(FILE *openFile, char searchTerm[])
{
    char lineBuffer[100];
    char *rString;

    //While the file is not EOF
    while (!feof(openFile))
    {
        fgets(lineBuffer, sizeof(lineBuffer), openFile);
        rString = strstr(lineBuffer, searchTerm);

        //If rString is not NULL, the line contains the search term
        if (rString != NULL)
        {
            //Return the line, minus the search term
            rString += strlen(searchTerm) + 1;
            printf("%s\n", rString);
            return;
        }
    }

    //Search term was not found
    printf(" NOT FOUND\n\n");
}

//Receives a time in seconds and formats to dd:hh:mm:ss
void printUpTime()
{
    //Constant variables
    const int SECONDS_IN_DAY = 86400;
    const int SECONDS_IN_HOUR = 3600;
    const int SECONDS_IN_MINUTE = 60;

    int days, hours, minutes, time;

    FILE *fp = fopen("/proc/uptime", "r");

    //If fp is null, print error and return
    if (fp == NULL)
    {
        fprintf(stderr, "/proc/uptime failed to open... does it exist?\n\n");
        return;
    }

    //Read first int
    fscanf(fp, "%d", &time);

    //Determine number of days in seconds
    days = (time / SECONDS_IN_DAY);
    time -= (SECONDS_IN_DAY * days);

    //Determine the number of hours left
    hours = (time / SECONDS_IN_HOUR);
    time -= (SECONDS_IN_HOUR * hours);

    //Determine the number of minutes left
    minutes = (time / SECONDS_IN_MINUTE);
    time -= (SECONDS_IN_MINUTE * minutes);

    //Print the formatted time
    printf("Time since last boot: %.2d:%.2d:%.2d:%.2d\n\n", days, hours, minutes, time);
    fclose(fp);
}

//Prints time in seconds all cpu's have spent in user mode, system mode, or idle
void printUSICPUTime()
{
    int totalUserMode;
    int totalUserModeNiced;
    int totalSystemMode;
    int totalIdle;

    FILE *openfile = fopen("/proc/stat", "r");

    //If openfile is null, print error and return
    if (openfile == NULL)
    {
        fprintf(stderr, "/proc/stat failed to open... does it exist?\n\n");
        return;
    }

    //Read user mode, niced user mode, system mode, and idle times
    fscanf(openfile, "%*s %d %d %d %d", &totalUserMode, &totalUserModeNiced, &totalSystemMode, &totalIdle);

    printf("CPU Time in User Mode (reported in seconds:): %.2f\n", (totalUserMode + totalUserModeNiced) / (float)sysconf(_SC_CLK_TCK));
    printf("CPU Time in System Mode (reported in seconds): %.2f\n", totalSystemMode / (float)sysconf(_SC_CLK_TCK));
    printf("Idle CPU Time (reported in seconds): %.2f\n\n", totalIdle / (float)sysconf(_SC_CLK_TCK));
    fclose(openfile);
}

//Prints the date/time the system was booted
void printBootDate()
{
    int temp;
    char linebuffer[100];

    FILE *openfile = fopen("/proc/stat", "r");

    //If openfile is null, print error and return
    if (openfile == NULL)
    {
        fprintf(stderr, "/proc/stat failed to open... does it exist?\n\n");
        return;
    }

    //While the file is not EOF
    while (!feof(openfile))
    {
        //Grab next string
        fscanf(openfile, "%s", linebuffer);

        //If string == "btime" next value is what we need
        if (!strcmp(linebuffer, "btime"))
        {
            //Print formatted time
            fscanf(openfile, "%d", &temp);
            time_t timeSinceEpoch = temp;
            printf("Boot Date: %s\n", ctime(&timeSinceEpoch));
            break;
        }
    }

    fclose(openfile);
}

//Prints an accumalted number of reads/writes performed on all disks in the system
void printNumberReadsWrites()
{
    int readsCompleted = 0;
    int writesCompleted = 0;
    int readTemp, writeTemp;
    char lineBuffer[200];

    //Open file, if file can't be opened return error
    FILE *openfile = fopen("/proc/diskstats", "r");

    if (openfile == NULL)
    {
        fprintf(stderr, "/proc/diskstats failed to open... does it exist?\n\n");
        return;
    }

    //While the file is not EOF
    while (!feof(openfile))
    {
        //Read formatted input, grab reads/writes completed successfully ignores all other fields
        fscanf(openfile, "%*s %*s %s %d %*d %*d %*d %d", lineBuffer, &readTemp, &writeTemp);

        //Only grabs disks (that have sd in the name) with the last letter not being a number
        if (strstr(lineBuffer, "sd") && isalpha(lineBuffer[strlen(lineBuffer) - 1]))
        {
            //Accumalate all read/write completed successfully across all devices in diskstats
            readsCompleted += readTemp;
            writesCompleted += writeTemp;
        }

        //Finish reading the line
        fgets(lineBuffer, sizeof(lineBuffer), openfile);
    }

    //Print accumalated answer and close file
    printf("Number of reads completed: %d\n", readsCompleted);
    printf("Number of writes completed: %d\n\n", writesCompleted);
    fclose(openfile);
}

//Prints total memory and total memory free
void printMemoryInfo()
{
    const float convFactor = 1024;
    int totalMem, memFree;

    FILE *openfile = fopen("/proc/meminfo", "r");

    //If file failed to open, print error and return
    if (openfile == NULL)
    {
        fprintf(stderr, "/proc/meminfo failed to open... does it exist?\n\n");
        return;
    }

    //Read the memory total, and memory free
    fscanf(openfile, "%*s %d %*s", &totalMem);
    fscanf(openfile, "%*s %d %*s", &memFree);

    //Print the result
    printf("Memory Configured: %.2fGB\n\n", totalMem / convFactor / convFactor);
    printf("Memory Free: %.2fGB\n\n", memFree / convFactor / convFactor);

    fclose(openfile);
}

//Prints the load avg (over 1 minute) to the console
float printSampledLoadAvg()
{
    float loadAvg;
    FILE *openfile = fopen("/proc/loadavg", "r");

    //If file was not opened, print an error and return
    if (openfile == NULL)
    {
        fprintf(stderr, "/proc/loadavg failed to open... does it exist?\n\n");
        return 0;
    }

    //Grab the current load avg (averaged over the last minute)
    fscanf(openfile, "%f", &loadAvg);
    printf("%.0f%% ", loadAvg * 100);

    //Flushing ouput to console
    fflush(stdout);

    fclose(openfile);
    return loadAvg;
}