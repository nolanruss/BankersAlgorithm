# BankersAlgorithm
**PROGRAMMING ASSIGNMENT 4**

Nolan Thompson
CSC 341, SP 2016
04/12/16

####DESCRIPTION
	C program implementing the bankers algorithm.  Parallel threads
	are "customers" attempting to access a set of shared resources.
	The program generates 5 customer threads, which respectively,
	attempt to claim a random quantity of resources based upon their
	individual needs.  Once a customer obtains the maximum number of
	resources, it sleeps for a random period of time, and then
	releases the resources.

	Each time a customer requests resources a function safety_test()
	is used to verify that the system is currently in a safe state
	prior to releasing the resources.

#####INSTRUCTIONS
	TO compile the program bankersAlgorithm.c, enter the following
	command in the command line terminal in the directory containing
	the file bankersAlgorithm.c

		gcc -std=c99 bankersAlgorithm.c -pthread

	Following compilation, enter the command below to run the
	program with initial resources of 10, 5, 7, and a runtime 50.

		./a.out 10 5 7 50

	All output will print to the console.

#####OBSERVATIONS
	The program performs correctly to prevent instances of deadlock.
	The randomly generated resource request values are based upon
	the needs of each process and what resources are available.  In
	certain instances, customer processes are allowed to submit
	requests for 0,0,0 due to the random nature of the request
	creation.	
