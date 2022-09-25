#include "Problem_A.h"

int main(int argc, char* argv[])
{
    int thread_count = argToInt(argv[1], strlen(argv[1]));
    INC = argToInt(argv[2], strlen(argv[2]));

    //Declare and allocate array for threads
    pthread_t* threads;
    threads = (pthread_t*)malloc(sizeof(pthread_t) * thread_count); //Threads points to a dynamically allocated array of threads.

    //Declare and allocate array for thread IDs
    int* ids;
    ids = (int*)malloc(sizeof(int) * thread_count);
    for(int i = 0; i < thread_count; i++)
        ids[i] = i;

    for(int i = 0; i < thread_count; i++)
    {
        pthread_create(&threads[i], NULL, &routine, &ids[i]);
    }

    for(int i = 0; i < thread_count; i++)
    {
        pthread_join(threads[i],NULL);
    }
    return 0;
}

void* routine(void* arg)
{
    int id = *(int*)arg;
    std::chrono::high_resolution_clock::time_point start, finish;
    std::chrono::duration<float> duration;
    start = std::chrono::high_resolution_clock::now();

    int c_high; //upper bound of C
    int c_low;  //lower bound of c
    int counter = 0;    //Stores number of triplets found.

    //Set bound for the each thread.
    c_low = id*INC;
    c_high = (id+1)*INC;

    int c;
    for(c = c_low; c < c_high; c++) //For all values of C (c_low <= c < c_high)
    {
        for(int a = 1; a < c; a++)  //For all values of A (1 <= a < c)
        {
            for(int b = 1; b < c; b++)  //For all values of B (1 <= b < c)
            {
                if((c*c) == ((a*a)+(b*b)))  //If triplet found
                counter++;  //Increment triplet counter
            }
        }
    }
    finish = std::chrono::high_resolution_clock::now(); //End timer
    duration = finish - start;  //Calculate duration
    int s = duration.count();   //Convert duration to integer in seconds.

    printf("id = %d, count = %d, elapsed time = %d seconds\n",id,counter,s);
    return 0;
}

int argToInt(char* &charArray, int charArray_size)
{
    int integerValue;
    char* newCharArray; //Create a new char[]
    newCharArray = (char*)malloc(sizeof(charArray_size) + 1);   //Dynamically allocate one additional space to hold a '\0' character.

    //Copy characters from argument over to newCharArray, and append the null terminator.
    for(int i = 0; i < charArray_size; i++)
        newCharArray[i] = charArray[i];
    newCharArray[charArray_size] = '\0';

    integerValue = strtol(newCharArray, NULL, 10);  //Convert string to integer

    free(newCharArray); //Deallocate memory from the heap
    return integerValue;
}
