=========================================================================================================

sim.c
    Main source code file for the program. This contains main(), and all the functions used to generate 
    output for all 3 simlulated scheduling algorithms

process.h
    Header file containing the struct for a process. This contains process information/statistics.

myTest.1
    Second input file that I used to test various combinations to verify my solutions

sampleRuns.txt
    Contains sample runs of all supported scheduling algorithms for the myTest.1 file

=========================================================================================================

How to compile (Option 1):
    Open a terminal inside the project directory and run the following command: "make"

    Note: If you want to clean the directory you can invoke "make clean"

How to compile (Option 2):
    Open a terminal inside the project directory and run the following command: "gcc sim.c -o sim"

=========================================================================================================

How to run:
    The program's 3 versions can be ran as follows

    FCFS: "./sim [input_file] FCFS"

    RR: "./sim [input_file] RR [time_quantum]"

    SRTF: "./sim [input_file] SRTF"

    Note: Captilization of the scheduling algorithm parameter is optional