/*
 * Implementation of simple barrier used to block tasks.
 *
 * Author: Rob Lyerly
 */
#include <librttest.h>
#include "util.h"
#include "rt_barrier.h"

/*
 * Initializes the specified barrier for the specified number of tasks
 */
int rt_barrier_init(rt_barrier* barrier, int num_tasks)
{
	barrier->task_count = num_tasks;
	atomic_set(0, &(barrier->cur_count));
	barrier->exit = FALSE;

	//Small optimization - if there is only 1 task, then we can avoid
	//all atomic gets/sets and simply return immediately for every barrier
	if(num_tasks == 1)
		barrier->single_task = TRUE;

	return SUCCESS;
}

/*
 * Blocking function that spins tasks until all have reached the barrier
 * or somebody has signaled that the tasks should exit the barrier
 */
int rt_barrier_wait(rt_barrier* barrier)
{
	//Atomically increment the number of tasks at the barrier and check
	//whether this task is responsible for re-setting cur_count to 0
	int clear_count = FALSE;
	int cur_num_tasks = atomic_inc(&(barrier->cur_count));
	if(cur_num_tasks == barrier->task_count)
		clear_count = TRUE;

	//Spin until all tasks have entered
	while((barrier->cur_count.counter < (barrier->task_count - 1)) && !barrier->exit) {}

	//If this task is responsible for clearing count, clear it
	if(clear_count && !barrier->exit)
		atomic_set(0, &(barrier->cur_count));

	//If we were signaled to exit, return as much
	if(barrier->exit) {
		atomic_add(-1, &(barrier->cur_count));
		return FAILURE;
	} else
		return SUCCESS;
}

/*
 * Signal all threads waiting at the barrier that they should exit
 */
void signal_exit(rt_barrier* barrier)
{
	if(!barrier->exit)
		barrier->exit = TRUE;
	return;
}

/*
 * Clear a previously sent exit signal
 */
void clear_signal_exit(rt_barrier* barrier)
{
	if(!barrier->exit)
		return;

	//Wait until all threads have exited, then clear the signal
	while(barrier->cur_count.counter < barrier->task_count) {}
	atomic_set(0, &(barrier->cur_count));
	return;
}
