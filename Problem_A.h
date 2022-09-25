#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <chrono>

//INC value to be set during main function
int INC;

//routine is the function to be executed by a thread upon creation. Receives the thread id via void casted int pointer
void* routine(void* arg);

//Receives a char array and its length and returns it converted to an integer.
int argToInt(char* &charArray, int charArray_size);
