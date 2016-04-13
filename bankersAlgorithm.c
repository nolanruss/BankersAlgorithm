/**
 * Banker's Algorithm (page 345)
 * Implementing the bankers algorithm.  
**/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>

#define TRUE 1
#define TOT_RESOURCES 3
#define TOT_PROCESSES 5 
#define MAX_SLEEP 5 
#define MIN 0  //minimum value of an element in buffer
#define MAX 1000 //maximum value of an element in buffer

pthread_mutex_t mutex;
sem_t empty;
sem_t full;

int buffer_time = 0;
int needState[TOT_PROCESSES][TOT_RESOURCES];
int workState[TOT_PROCESSES][TOT_RESOURCES];
int maxWorkState[TOT_PROCESSES][TOT_RESOURCES];
int availableResources[TOT_RESOURCES];
int emptyResources = 0;
int processNum[TOT_PROCESSES];
int runtime = 0;

void *customer(void *param);

int myRand() {
   return rand() % (MAX - MIN + 1) + MIN;
}

/* Print out the workState, needDate, maxState, and available resources. */	
int printState(){
		printf("\n<Available resources>\n");
		printf("%d %d %d\n", availableResources[0], availableResources[1],availableResources[2]);
		printf("<workState> <needState> <maxState>\n");
		for (int i=0; i < TOT_PROCESSES; i++){
			printf("%d  %d  %d       ", workState[i][0], workState[i][1], workState[i][2]);
			printf("%d  %d  %d       ", needState[i][0], needState[i][1], needState[i][2]);
			printf("%d  %d  %d", maxWorkState[i][0], maxWorkState[i][1], maxWorkState[i][2]);
			printf("\n");
		}
}

/*
 * Check if the system is in a safe state. 
 */
int safety_test(int tmpWS[TOT_PROCESSES][TOT_RESOURCES],int tmpAR[TOT_RESOURCES]){
	int finishState[TOT_PROCESSES] = {0, 0, 0, 0, 0};
	int finishCount = 0;

	/* Cycle through each process to determine the safe state. */
	/* It may take up to nxn comparisons to find the safe state.*/
	for (int k=0; k < TOT_PROCESSES; k++){
		for (int i=0; i < TOT_PROCESSES; i++){
			//printf("\nIteration %d\n", i);
			if (!finishState[i]){
				int resourceCount = 0;

				/* Check whether the need for resources can be met. */
				for (int j=0; j < TOT_RESOURCES; j++){
					if ((maxWorkState[i][j] - tmpWS[i][j]) <= tmpAR[j]){
						resourceCount++;	
					}
				}
				
				/* If the need can be met, set the state as finished and
				 * add the resources to the available resources tmpAR */
				if (resourceCount == TOT_RESOURCES){
					finishState[i] = 1;
					finishCount++;
					for (int j=0; j < TOT_RESOURCES; j++){
						tmpAR[j] = tmpAR[j] + tmpWS[i][j];
					}
				}
			}
			/* Safe state found, return 1. */
			if (finishCount == TOT_PROCESSES){
				return 1;
			}
		}	
	}	
	/* Safe state not found, return 0. */
	return 0;
}

int init_StartState(FILE* st){
	int i = 0; // Process index#
	char c = ',';
	/* Get the work values from max_demand.txt */
	for (int j=0; j < (TOT_PROCESSES*TOT_RESOURCES) ; j++){
		if (!(EOF == fscanf(st, "%d%c", &i, &c))){
			workState[j/TOT_RESOURCES][j%TOT_RESOURCES] = 0;
			maxWorkState[j/TOT_RESOURCES][j%TOT_RESOURCES] = i;
			needState[j/TOT_RESOURCES][j%TOT_RESOURCES] = i;
		}	  		
	}
	return 1;
}

/*											 
 * Release all resources held by a process.  
 */											 
int release_resources(int pNum)
{
	/* Acquire the mutex lock. */
	pthread_mutex_lock(&mutex);
	buffer_time += 1; // Increment the time.

	printf("\nTime %d Customer# %d releasing ", buffer_time, pNum); 

	/* Add resources back to availableResources. */
	for (int i=0; i < TOT_RESOURCES; i++){
		printf("%d ", maxWorkState[pNum][i]);
		availableResources[i] = availableResources[i] + maxWorkState[pNum][i];
		needState[pNum][i] = maxWorkState[pNum][i];
		workState[pNum][i] = 0;
	}
	printf("\n");

	/* Release the mutex lock. */
	pthread_mutex_unlock(&mutex);
	return 1;
}

/*
 * request_resources function takes an array of resources being requested
 * and checks whether the proposed new resource states are safe. 
 */
int request_resources(int pNum)
{
	/* Acquire the mutex lock. */
	pthread_mutex_lock(&mutex);

	int rRequest[TOT_RESOURCES];
	emptyResources = 0; // Counts the number of rec. at 0.

	buffer_time += 1; // Increment the time.

	for (int i=0; i < TOT_RESOURCES; i++){
		if (needState[pNum][i] != 0){
			rRequest[i] = rand() % needState[pNum][i] + 1;
		}
		else {
			rRequest[i] = 0;
		}
		/* If the request is too high, set as max avail.*/
		if (rRequest[i] > availableResources[i]){
			rRequest[i] = availableResources[i];
		}
		if (availableResources[i] == 0){
			emptyResources ++;	
		}
	}

	if (emptyResources == TOT_RESOURCES){
		/* Release mutex lock */
		pthread_mutex_unlock(&mutex);
		return 0;
	}

	/* Generate the proposed new work state */
	int tmpWorkState[TOT_PROCESSES][TOT_RESOURCES];
	int tmpAvailRec[TOT_RESOURCES];
	
	/* Create a temporary work state and resource request. */
	for (int i=0; i < TOT_PROCESSES; i++){
		for (int j=0; j < TOT_RESOURCES; j++){
			tmpWorkState[i][j] = workState[i][j];
			if (i == pNum){
				tmpWorkState[i][j] = tmpWorkState[i][j] + rRequest[j];
				tmpAvailRec[j] = availableResources[j] - rRequest[j];
			}
		}
	}

	printf("\nTime %d  Customer# %d\n", buffer_time, pNum);
	printf("		Requesting: %d %d %d\n", rRequest[0], rRequest[1], rRequest[2]);
	printf("		Available:  %d %d %d\n", availableResources[0], availableResources[1], availableResources[2]);

	/* If the new work state is safe, then allocate resources. */
	if (safety_test(tmpWorkState, tmpAvailRec)){
		int fullResCount = 0; // Count the number of full resources.
		for (int i=0; i < TOT_RESOURCES; i++){
			availableResources[i] = availableResources[i] - rRequest[i];
			workState[pNum][i] = workState[pNum][i] + rRequest[i];
			needState[pNum][i] = maxWorkState[pNum][i] - workState[pNum][i];
			fullResCount = fullResCount + (needState[pNum][i] == 0);
		}

		/* Print the result. */
		printf("		Customer using resources...\n");
		printf("		Allocated:  %d %d %d\n", rRequest[0], rRequest[1], rRequest[2]); 
		printf("		Available:  %d %d %d\n", availableResources[0], availableResources[1], availableResources[2]);
		//printState();

		/* Release the mutex lock. */
		pthread_mutex_unlock(&mutex);
		return 1;
	}	
	else{
		printf("		INSUFFICIENT RESOURCES\n");
		printf("		Customer# %d denied to avoid deadlock\n", pNum);

		/* Release the mutex lock. */
		pthread_mutex_unlock(&mutex);
		return 0;
	}

	/* Release the mutex lock. */
	pthread_mutex_unlock(&mutex);
	return 0;
}

int main(int argc, char *argv[])
{
	int sleepTime, customerThreads;
	int j;
	FILE* stream = fopen("max_demand.txt", "r");
	
	if(argc != 5)
	{
		fprintf(stderr, "Useage: <Resource 1 quantity> <Resource 2 quantity> <Resource 3 quantity> <Runtime>\n");
		return -1;
	}
	else{
		runtime = atoi(argv[4]);
		availableResources[0] = atoi(argv[1]);
		availableResources[1] = atoi(argv[2]);
		availableResources[2] = atoi(argv[3]);
	}
	if(!init_StartState(stream)){
		fprintf(stderr, "File max_demand.txt not found.");
	}

	/* 5 customer threads. */
	customerThreads = TOT_PROCESSES; 

	/* Initialize the synchronization tools */
	printf("%d\n",pthread_mutex_init(&mutex, NULL));
	srand(time(0));

	/* Create the customer threads */
	for(j = 0; j < customerThreads; j++)
	{
		processNum[j] = j;
		pthread_t tid;
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_create(&tid, &attr, customer, &processNum[j]);
	}
	pthread_exit(NULL);
	return 0;
}

/*
 * Each thread that calls *customer is assigned an integer process number.
 * The thread uses this process number to make resource requests and release
 * resources.
 */

void *customer(void *param) { 
	int pNum = *((int*) param);
	int resReq[TOT_RESOURCES];
	int r;
	int fullResCount = 0;

	while(TRUE)
	{
		r = rand() % MAX_SLEEP;
		sleep(r); // Sleep for both holding and requesting resources.
		/* Release resources if maximum resources are satisfied. */
		if (fullResCount == TOT_RESOURCES){		
			if (release_resources(pNum)){
				fullResCount = 0;
			}		
			else{
				fprintf(stderr, "Error releasing resources");
			}
		}

		/* Still need resources, create a random request of resources */
		else{
			if (request_resources(pNum)){
				fullResCount = 0;

				/* Check whether all needs are met. */
				for (int i=0; i < TOT_RESOURCES; i++){
					fullResCount = fullResCount + (needState[pNum][i] == 0); 
				}	
			}
		}
		if (buffer_time >= runtime){
			break;	
		}
	}
}
