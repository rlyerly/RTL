/*
 * Cyclic test
 *
 * Measures the delay between when a periodic task is supposed to be awoken
 * and when that task is actually awoken.
 *
 * Author: Rob Lyerly
 */
#include <stdio.h>
#include <sys/sysinfo.h>
#include <librttest.h>
#include "util.h"
#include "rt_barrier.h"

#ifndef gettid
#define gettid() (long int)syscall(224)
#endif

/* Cyclic test parameters.  Times are in ns */
#define PERIOD 100000
#define DEADLINE 100000
#define EXECTIME 10000
#define UTIL 50	//TODO what is util?

/* Global statistics counters */
static long** statistics;

/* Structure used to pass configuration parameters to the thread */
typedef struct _threadParam {
	int numLoops;
	int threadNum;
	int priority;
} threadParam;

/* Global value set by any of the threads if they encounter a problem */
static int threadRetVal = 0;

/* Barrier used to release all threads at the same time */
static rt_barrier barrier;

/*
 * Thread function which performs the cyclictest.  Sets up the real-time
 * parameters and changes the thread to a real-time task as necessary.
 */
static void* cyclicTestFunction(void* param)
{
	threadParam* myParam = (threadParam*)param;
	struct timespec period, deadline;
	period_param myPeriod;
	RTIME expectedWakeup = 0;
	RTIME firstTime = 0;
	int i = 0;

	//TODO Block to make sure all threads reach this point
	//rt_barrier_wait(&barrier);

	//Make the task periodic
	nsec_to_ts(PERIOD, &period);
	rt_task_begin(EXECTIME, PERIOD, &myPeriod, TRUE, gettid());

	//Calculate the first deadline
	expectedWakeup = rt_gettime() + PERIOD;
	nsec_to_ts(expectedWakeup, &deadline);

	//Throw away the first period
	rt_job_begin(myParam->priority, UTIL, &deadline, &period, EXECTIME,
			&myPeriod, TRUE);
	rt_job_end(LOW_PRIO);

	firstTime = rt_gettime();
	expectedWakeup += PERIOD;
	nsec_to_ts(expectedWakeup, &deadline);
	
	//Perform the test
	for(i = 0; i < myParam->numLoops; i++)
	{
		rt_job_begin(myParam->priority, UTIL, &deadline, &period,
				EXECTIME, &myPeriod, TRUE);

		statistics[myParam->threadNum][i] = rt_gettime();

		expectedWakeup += PERIOD;
		nsec_to_ts(expectedWakeup, &deadline);

		rt_job_end(LOW_PRIO);
	}

	//Calculate the delay based on sampled values
	for(i = myParam->numLoops - 1; i > 0; i--)
	{
		statistics[myParam->threadNum][i] =
			statistics[myParam->threadNum][i] -
			statistics[myParam->threadNum][i - 1] -
			PERIOD;
	}
	statistics[myParam->threadNum][0] = statistics[myParam->threadNum][0] -
			firstTime - PERIOD;

	//End the real-time section
	rt_task_end(&myPeriod, TRUE);

	return NULL;
}

/* Main test entry point.  Forks the threads (which actually run the test),
 * waits for them to join and prints the results to file.
 */
int cyclictest(int numTasks, task tasks, int affinity, int numLoops, char* fn)
{
	threadParam* param;
	int i = 0, j = 0;

	//Go ahead and open the file.  This way, we avoid the situation where
	//we spend a lot of time running the test only for it to be unable to
	//record the results
	FILE* fp = fopen(fn, "w");
	if(!fp)
	{
		fprintf(stderr, "ERROR: Could not open file \"%s\"!\n", fn);
		return -1;
	}

	//Initialize a barrier used by the tasks
	//rt_barrier_init(&barrier, numTasks);

	//Allocate data structures
	param = (threadParam*)calloc(numTasks, sizeof(threadParam));
	statistics = (long**)calloc(numTasks, sizeof(long*));
	for(i = 0; i < numTasks; i++)
		statistics[i] = (long*)calloc(numLoops, sizeof(long));

	//Create the tasks, which run cyclictest
	for(i = 0; i < numTasks; i++, tasks++) {
		param[i].numLoops = numLoops;
		param[i].threadNum = i;
		param[i].priority = tasks->prio;

#if KERN_SCHED_DEAD
		//EDF is strange in that you must set the affinity for a task
		//to all cores to set the scheduler to EDF
		CPU_ZERO(&(tasks->cpus_allowed));
		for(j = 0; j < get_nprocs(); j++)
			CPU_SET(j, &(tasks->cpus_allowed));
#else
		CPU_ZERO(&(tasks->cpus_allowed));
		CPU_SET(i, &(tasks->cpus_allowed));
#endif //KERN_SCHED_DEAD

		tasks->func = cyclicTestFunction;
		tasks->arg = (void*)(&param[i]);

		printf("Task characteristics: %p %p %s %d\n", tasks->func,
				tasks->arg, tasks->name, tasks->prio);

		if(task_create(tasks))
		{
			fprintf(stderr, "ERROR: Could not create tasks!\n");
			return -1;
		}
	}

	//Wait for all tasks to finish and join
	task_join_all();

	//Print results to file
	for(i = 0; i < numTasks; i++)
	{
		for(j = 0; j < numLoops; j++)
		{
			fprintf(fp, "%ld ", statistics[i][j]);
		}
		fprintf(fp, "\n");
	}
	fclose(fp);

	//Cleanup
	for(i = 0; i < numTasks; i++)
		free(statistics[i]);
	free(statistics);
	free(param);

	return threadRetVal;
}
