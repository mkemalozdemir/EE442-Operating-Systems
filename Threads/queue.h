#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>             /*Necessary library includes*/
#include <math.h>
#include <unistd.h>
#include <time.h>

#define DEFAULT_NUMBER_WORKERS          3
#define DEFAULT_MAX_SIZE                5
#define DEFAULT_AMOUNT_RANDOM_NUMBERS   10      /*Default values of some properties*/
#define DEFAULT_LOWER_BOUND             1
#define DEFAULT_UPPER_BOUND             2000
#define DEFAULT_RATE_GENERATION         100


typedef struct NumberQueue
{
    int *ipQ;           /*Address of the queue*/
    int iMaxSizeQ;
    int iCurrentSizeQ;          /*Safe Q struct*/
    int iFront;
    int iRear;
}Queue;

extern char *optarg;   /*To be able to use getopt() function, externed from the unistd.h*/

void QueueInitialize (Queue* queue, int iSize); /*Initialize the queue*/
void QueueInsert (Queue* queue, int iValue);    /*Insert the input to the queue*/
int QueueRemove (Queue* queue);                 /*Delete the item in the front of the queue*/
void QueueDestroy (Queue* queue);               /*Deletes the ipQ to prevent memory leak*/

double expGenerator(double dRate);              /*Generates the time for sleeping*/
int isPrime(int num);                           /*Checks the prime number*/

void* NumGenerator(void* ptr);                  /*Number generator thread function*/
void* Worker(void* ptr);                        /*Worker thread function*/