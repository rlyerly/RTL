/*
 * Entry point for cyclictest
 *
 * Author: Rob Lyerly <rlyerly@vt.edu>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <librttest.h>
#include "util.h"

/* TODO remove when going back to regular rt_init */
#include <sys/mman.h>

/*
 * Help text
 */
const char* help = "Cyclictest - tests the time difference between when a \
periodic, real-time application is supposed to be awoken and when it is \
actually awoken.\n\nArguments:\n \
\t-h	: print this help\n \
\t-t	: number of tasks\n \
\t-l	: number of loops\n \
\t-m	: migrate threads (don't affine threads to cores)\n \
\t-p	: scheduling policy [fifo | rr]\n \
\t-f	: filename in which to record results\n\n";

/*
 * Prototype for cyclic test, contained in "cyclictest.c"
 */
int cyclictest(int numTasks, task tasks, int affinity, int numLoops, char* fn);

/*
 * Library version of this segfaults, for now we're going to copy relevant
 * parts here...
 * TODO remove
 */
void temp_rt_init()
{
	//Lock memory
	mlockall(MCL_CURRENT | MCL_FUTURE);

#if KERN_LITMUS
	init_litmus();
#elif KERN_CHRONOS
	struct sched_param param, old_param;

	sched_getparam(0, &old_param);
	param.sched_priority = MAIN_PRIO;

	if (sched_setscheduler(0, SCHED_FIFO, &param) == -1) {
		printf("sched_setscheduler() failed.");
		exit(1);
	} else
		printf("[ChronOS] sched_fifo scheduler is set\n");

	if (set_scheduler(SET_SCHED_ALGO | SET_SCHED_FLAG, SET_SCHED_PRIO, SET_SCHED_PROC)) {
		printf("Selection of RT scheduler failed! Is the scheduler loaded?");
		exit(1);
	} else
		printf("[ChronOS] RT scheduler is set\n");
#endif
}

/*
 * Temporary cleanup function
 * TODO remove
 */
void temp_rt_cleanup()
{
	munlockall();
}

/*
 * Tunable parameters and command-line argument parsing function
 */
static int numTasks = 1;
static int numLoops = 1000;
static int affinity = AFFINITY;
static int policy = SCHED_FIFO;
static char* filename = "cyclictest_results.txt";
static int priority = HIGH_PRIO;

int parse_args(int argc, char** argv)
{
	int arg = 0;
	char* policyString = NULL;

	while((arg = getopt(argc, argv, "hmt:l:p:f:")) != -1)
	{
		switch(arg) {
		case 'h':
			printf("%s", help);
			exit(0);
		case 'm':
			affinity = NO_AFFINITY;
			break;
		case 't':
			numTasks = atoi(optarg);
			if(numTasks < 1)
				numTasks = 1;
			break;
		case 'l':
			numLoops = atoi(optarg);
			if(numLoops < 1)
				numLoops = 1;
			break;
		case 'p':
#ifdef KERN_EDF
			printf("WARNING: Scheduling policy will be set to SCHED_DEADLINE\n");
#endif
			policyString = optarg;
			break;
		case 'f':
			filename = optarg;
			break;
		case ':':
			if(optopt == 't') {
				printf("You must specify the number of threads\n");
				exit(1);
			} else if(optopt == 'l') {
				printf("You must specify the number of loops\n");
				exit(1);
			} else if(optopt == 'p') {
				printf("You must specify a scheduling policy [fifo | rr]\n");
				exit(1);
			} else if(optopt == 'f') {
				printf("You  must specify a filename\n");
				exit(1);
			} else {
				printf("Unknown argument...\n");
			}
			break;
		case '?':
			if(optopt == 't') {
				printf("You must specify the number of threads\n");
				exit(1);
			} else if(optopt == 'l') {
				printf("You must specify the number of loops\n");
				exit(1);
			} else if(optopt == 'p') {
				printf("You must specify a scheduling policy [fifo | rr]\n");
				exit(1);
			} else if(optopt == 'f') {
				printf("You  must specify a filename\n");
				exit(1);
			} else {
				printf("Unknown argument...\n");
			}
			break;
		default:
			printf("Unknown argument...\n");
			break;
		}
	}

	//Set the scheduling policy
	if(policyString && strstr(policyString, "rr"))
		policy = SCHED_RR;


	printf("\nRunning the test with the following parameters:\n");
	printf("\tNumber of tasks: %d\n", numTasks);
	printf("\tNumber of periodic loops: %d\n", numLoops);
	printf("\tAffine tasks to cores: %s\n", (affinity == AFFINITY ? "yes" : "no"));
#ifdef KERN_EDF
	printf("\tScheduling policy: SCHED_DEADLINE\n");
#else
	printf("\tScheduling policy: %s\n", (policy == SCHED_FIFO ? "SCHED_FIFO" : "SCHED_RR"));
#endif
	printf("\tRecord results in file: %s\n\n", filename);

	return SUCCESS;
}

/*
 * Main function
 */
int main(int argc, char** argv)
{
	int i = 0;
	char** names;

	//Parse the command-line arguments and print setup
	temp_rt_init(/*"", NULL, 0, NULL TODO uncomment*/);
	//rt_init("", NULL, 0, NULL);
	parse_args(argc, argv);

	//Send signals to prepare application
	setup();

	//Setup the tasks
	names = (char**)malloc(sizeof(char*) * numTasks);
	for(i = 0; i < numTasks; i++)
		names[i] = (char*)malloc(sizeof(char) * 100);

	task curTask = taskset_init(numTasks);
	task startTask = curTask;
	for(i = 0; i < numTasks; i++, curTask++)
	{
		//Set the task's general parameters
		sprintf(names[i], "%s %d", "Task", i);
		curTask->name = names[i];
		curTask->policy = policy;
		curTask->prio = priority;
		curTask->max_msg_size = 0;
		curTask->stack_size = 0;
	}

	//Run the tests
	int testRetVal = cyclictest(numTasks, startTask, AFFINITY, numLoops,
			filename);

	//Cleanup
	for(i = 0; i < numTasks; i++)
		free(names[i]);
	free(names);

	temp_rt_cleanup();

	if(testRetVal) {
		printf("Test was unsuccessful\n\n");
		return FAILURE;
	} else {
		printf("Successfully finished test and recorded results in \"%s\"\n\n", filename);
		return SUCCESS;
	}
}
