#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

//Function prototypes
int FIFO();
void loadFile(char fileName[]);
int LRU(int frames[], int index);
int OPT(int frames[], int index);
void pageReplacement(char algorithm[]);
int frameContainsPage(int frames[], int request, int loadedPages);

//Globals
int numPages, numFrames, numRequests, fifoReplacement = 0;
int* requests;

int main(int argc, char* argv[])
{
    int index;

    //Make sure the number of arguments passed is correct, if not abort...
    if (argc < 3 || argc > 3)
    {
        fprintf(stderr, "Usage: proj4 input_file [FIFO|LRU|OPT]\n");
        exit(1);
    }

    //Enforce uppercase on passed paramemter
    for (index = 0; index < strlen(argv[2]); index++)
    {
        argv[2][index] = toupper(argv[2][index]);
    }

    //Make sure the parameter passed is a valid algorithm
    if (strcmp(argv[2], "FIFO") && strcmp(argv[2], "LRU") && strcmp(argv[2], "OPT"))
    {
        fprintf(stderr, "Usage: proj4 input_file [FIFO|LRU|OPT]\n");
        exit(1);
    }

    //Load, the file, perform the algorithm, and free the allocated memory
    loadFile(argv[1]);
    pageReplacement(argv[2]);
    free(requests);
}

//Generic function that performs page replacement based on the name of the passed algorithm
void pageReplacement(char algorithm[])
{
    int loadedPages = 0, pageFaults = 0;
    int nextReplacement, index;

    //Dynamically allocate an array to hold the pages in memory
    int* frames = calloc(numFrames, sizeof(int));

    //Iterate through the request and perform page replacement
    for (index = 0; index < numRequests; index++)
    {
        //If the array contains the page, returns the index of the frame. Else -1
        int alreadyInFrames = frameContainsPage(frames, requests[index], loadedPages);

        if (alreadyInFrames != -1)
        {
            printf("Page %d already in Frame %d\n", requests[index], alreadyInFrames);
        }

        //All the frames are full, must perform replacement based on the desired algorithm
        else if (loadedPages >= numFrames)
        {
            if (strcmp(algorithm, "FIFO") == 0)
            {
                nextReplacement = FIFO();
            }
            else if (strcmp(algorithm, "LRU") == 0)
            {
                nextReplacement = LRU(frames, index);
            }
            else
            {
                nextReplacement = OPT(frames, index);
            }            

            //Perform the replacement based on the returned page to replace.
            printf("Page %d unloaded from Frame %d, ", frames[nextReplacement], nextReplacement);
            printf("Page %d loaded into Frame %d\n", requests[index], nextReplacement);
            frames[nextReplacement] = requests[index];
            pageFaults++;
        }

        //A frame is empty, load request into frame
        else
        {
            printf("Page %d loaded into Frame %d\n", requests[index], loadedPages);
            frames[loadedPages] = requests[index];
            loadedPages++;  
            pageFaults++;        
        }     
    }

    //Print the number of faults and free the allocated memory    
    printf("%d page faults\n", pageFaults);
    free(frames);
}

//Chooses the next page to be replaced based on the FIFO algorithm
int FIFO()
{
    int temp = fifoReplacement;
    fifoReplacement = (fifoReplacement + 1) % numFrames;
    return temp;    
}

//Choose the next page to be replaced based on the LRU algorithm
int LRU(int frames[], int index)
{
    int count, location, nextReplacement, age = -1, tempAge = 1;    

    //For each frame, find the oldest page, return the index of the oldest page in frame
    for (count = 0; count < numFrames; count++)
    {
        //Iterate backwards through the requests to determine the age of each page in frame
        for(location = index - 1; location >= 0; location--)
        {
            //Found where the current page in frame was loaded
            if (requests[location] == frames[count])
            {
                //If the age of this frame is older than the currently stored age save it
                if (tempAge == (tempAge > age ? tempAge : age))
                {
                    age = tempAge;
                    nextReplacement = count;
                    break;
                }
                break;
            }
            tempAge++;
        }
        tempAge = 1;
    }

    //Return the index of the next replacement in frames
    return nextReplacement;  
}

int OPT(int frames[], int index)
{
    int count, location, nextReplacement, nextUse = -1, tempNextUse = 1;    

    //For each frame, find the page next used last and return its index
    for (count = 0; count < numFrames; count++)
    {
        //Iterate forwards through the requests to determine the next use of each page
        for(location = index + 1; location < numRequests; location++)
        {
            //Found where the current page in frame will be next used
            if (requests[location] == frames[count])
            {
                //If the next use of this frame is greater than previous one stored, save it
                if (tempNextUse == (tempNextUse > nextUse ? tempNextUse : nextUse))
                {
                    nextUse = tempNextUse;
                    nextReplacement = count;
                    break;
                }
                break;
            }

            //If the page won't be used again, return it as the next replacement
            if (location == (numRequests - 1))
            {
                return count;
            }
            tempNextUse++;
        }
        tempNextUse = 1;
    }

    //Return the index of the next replacement in frames
    return nextReplacement;
}

//Loads the passed filename into memory
void loadFile(char fileName[])
{
    //Open the file
    FILE* fp = fopen(fileName, "r");
    int index;

    //If the file failed to open... inform the user
    if (fp == NULL)
    {
        fprintf(stderr, "%s failed to open... Does it exist?\nExiting Program...\n", fileName);
        exit(1);
    }

    //Read the first line of the file to get the statistical information
    fscanf(fp, "%d %d %d", &numPages, &numFrames, &numRequests);

    requests = calloc(numRequests, sizeof(int));

    //Read the requests into the requests array
    for (index = 0; index < numRequests; index++)
    {
        fscanf(fp, "%d", &requests[index]);
    }    
    fclose(fp);
}

//Checks to see if the page is already loaded, if so it returns the frame number. Else, it returns -1
int frameContainsPage(int frames[], int request, int loadedPages)
{
    int index;

    for (index = 0; index < loadedPages; index++)
    {
        if (frames[index] == request)
        {
            return index;
        }
    }    
    return -1;
}