#include "queue.h"

Queue Q;                                            //Create Q from the queue struct

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER; //mutex declaration and initialization
//pthread_cond_t cond = PTHREAD_COND_INITIALIZER; 
int finish = 0;                                     //Flag to end program
int nextThread = 0;
int iNumWorkerThreads = DEFAULT_NUMBER_WORKERS;
int iSizeQMaximum = DEFAULT_MAX_SIZE;
int iAmountRandNums = DEFAULT_AMOUNT_RANDOM_NUMBERS;    //Properties are declared with default values
int iLowerBound = DEFAULT_LOWER_BOUND;
int iUpperBound = DEFAULT_UPPER_BOUND;
int iGenRate = DEFAULT_RATE_GENERATION;

int main(int argc, char *argv[]) {
    int i;
    int op;
    srand(time(NULL));
    /*Acording to the input properties are arranged below*/
    while ((op = getopt(argc, argv, "t:q:r:m:n:g:")) != -1) {
        switch (op) {
            case 't':
                iNumWorkerThreads = atoi(optarg);
                break;

            case 'q':
                iSizeQMaximum = atoi(optarg);
                break;

            case 'r':
                iAmountRandNums = atoi(optarg);
                break;

            case 'm':
                iLowerBound = atoi(optarg);
                break;

            case 'n':
                iUpperBound = atoi(optarg);
                break;

            case 'g':
                iGenRate = atoi(optarg);
                break;

            default: continue;
        }
    }
    printf("GENERATION_RATE: %d\n", iGenRate); /*Generation rate is printed here*/
    pthread_t WorkerThread[iNumWorkerThreads]; /*Thread declarations*/
    pthread_t numGenThread;       
    QueueInitialize(&Q, iSizeQMaximum);         /*Initialization of the queue is done*/

    /*Threads are created below*/
    if (pthread_create(&numGenThread, NULL, NumGenerator, NULL) != 0) {
        printf("Error creating number generator thread\n");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < iNumWorkerThreads; i++) {
        
        int* threadIndex = (int*)malloc(sizeof(int));
        *threadIndex = i;
        if (pthread_create(&WorkerThread[i], NULL, Worker, (void*)threadIndex) != 0) {
            printf("Error creating worker thread %d\n", i);
            exit(EXIT_FAILURE);
        }
    }
    /*Threads are joined, so until they are terminated, main thread does not continue*/
    pthread_join(numGenThread, NULL);
    for (i = 0; i < iNumWorkerThreads; i++) {
        pthread_join(WorkerThread[i], NULL);
    }
    /*Destroy the queue and thredIndex to prevent memory leak*/
    QueueDestroy(&Q);

    return 0;
}


void QueueInitialize (Queue* queue, int iSize)
{   
    queue->ipQ = (int*)malloc(iSize * sizeof(int)); /*Dynamic memory allocation for queue*/
    if (queue ->ipQ == NULL)
    {
        exit(1);
    }

    queue->iMaxSizeQ = iSize;                       /*Other items of the struct initialized*/   
    queue->iCurrentSizeQ = 0;
    queue->iFront = 0;
    queue->iRear = 0; 
}

void QueueInsert (Queue* queue, int iValue)
{
    queue->ipQ[queue->iRear] = iValue;                    /*Add the new item to rear*/
    queue->iRear = (queue->iRear + 1) % queue->iMaxSizeQ; /*Increase the rear, cyclic queue*/
    queue->iCurrentSizeQ++;                               /*Increase the number of item in the queue*/
}

int QueueRemove (Queue* queue)
{
    int iRemovedItem;
    iRemovedItem = queue->ipQ[queue->iFront];             
    queue->iFront = (queue->iFront+1) % queue->iMaxSizeQ; /*Remove the item in the front*/
    queue->iCurrentSizeQ--;                               /*Decrease the number of item in the queue*/
    return iRemovedItem;                                  /*Return the removed item*/
}

void QueueDestroy (Queue* queue)
{
    free(queue->ipQ);                                     /*Delete the queue*/
}

double expGenerator(double dRate)
{
    double dHolder = (double)rand()/RAND_MAX;             /*x in the formula*/
    return ((-log(1-dHolder))/dRate);                     /*(1/lambda)*ln(1-x)*/
}

int isPrime(int num)
{
    int i;
    if(num <= 1)                    /*Numbers less than 2 cannot be prime number*/
    {
        return 0;
    }
    for(i=2; i <= sqrt(num); i++)   /*This is a common mathematical method to detect prime numbers*/
    {
        if(num%i == 0)
        {
            return 0;
        }
    }
    return 1;                       /*Return 1 if number is prime, return 0 if not*/
}

void* NumGenerator(void* ptr) {
    int cnt = 0;
    int number;
    double T;
    while (cnt < iAmountRandNums) { /*Stop after generating the desired number of number*/
        number = rand() % (iUpperBound - iLowerBound + 1) + iLowerBound;
        T = expGenerator((double) iGenRate);
        sleep(T);   /*Sleep random times*/

        pthread_mutex_lock(&mutex1);
        if (Q.iCurrentSizeQ < Q.iMaxSizeQ) {    /*If queue has space insert*/
            QueueInsert(&Q, number);
            cnt++;
        }
        pthread_mutex_unlock(&mutex1);
    }
    finish = 1; /*Set the flag*/
    pthread_exit(NULL); /*Exit the thread*/
}


void* Worker(void* ptr) {
    int number;
    int Index = *((int*)ptr); 
    while (1) 
    {
        while(Index != nextThread)  /*Used as semaphores to obtain fair share scheduling*/
        {
            if(Q.iCurrentSizeQ == 0 && finish)
            {
                pthread_exit(NULL);
            }
        }
        pthread_mutex_lock(&mutex1);

        if (Q.iCurrentSizeQ > 0) {
            number = QueueRemove(&Q);   /*Remove the item from queue*/
            printf("Thread ID: %ld, Number: %d ", pthread_self(), number);
            if (isPrime(number)) { /*check prime number*/
                printf("is a prime number.\n");
            } else {
                printf("is not a prime number. Divisors: ");
                int cnt = 0;
                for (int i = 1; i <= number; i++) {
                    if (number % i == 0) {
                        printf("%d ", i);
                        cnt++;
                    }
                    if (cnt >= 10) {
                        break;
                    }
                }
                printf("\n");
            }
            pthread_mutex_unlock(&mutex1);  /*release mutex*/
            nextThread = (Index+1) % iNumWorkerThreads; /*Signal the next thread*/
        } else {
            pthread_mutex_unlock(&mutex1);
            if (finish) 
            {
                break;  /*If there is no item in the queue and generator thread exit, exit the loop and thread*/
            }
        }
    }
    pthread_exit(NULL);
}