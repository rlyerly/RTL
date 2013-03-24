/*
 * Simple barrier used to block tasks until all are ready to be released.
 * This is a spin-barrier, in that tasks spin until they are unblocked,
 * consuming computational resources.
 *
 * Author: Rob Lyerly <rlyerly@vt.edu>
 */

/*
 * Barrier object
 */
typedef struct _rt_barrier {
	int task_count;
	atomic_t cur_count;
	int exit;
	int single_task;
} rt_barrier;

/*
 * Barrier interface
 */
int rt_barrier_init(rt_barrier* barrier, int num_tasks);
int rt_barrier_wait(rt_barrier* barrier);
void signal_exit(rt_barrier* barrier);
void clear_signal_exit(rt_barrier* barrier);
