How to compile (Option 1):
    Open up a terminal inside the project directory and run the following command: "make"

    Note: If you want to clean the directory you can invoke: "make clean"

How to compile (Option 2):
    Open up a terminal inside the project directory and run the following command: "gcc observer.c -o observer"

How to run:
    The program's 3 versions can be ran as follows

    Standard Report: "./observer"

    Short Report: "./observer -s"

    Long Report: "./observer -l [interval] [duration]"
    
    NOTE: replace [interval] and [duration] with two integers, one defining the interval by which load average will be sampled, and the other
    defining the duration of sampling