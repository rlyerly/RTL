/******************************************************************************
 *
 *   Copyright © International Business Machines  Corp., 2006-2008
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * NAME
 *       librttest.h
 *
 * DESCRIPTION
 *      A set of commonly used convenience functions for writing
 *      threaded realtime test cases.
 *
 * USAGE:
 *       To be included in testcases.
 *
 * AUTHOR
 *        Darren Hart <dvhltc@us.ibm.com>
 *
 * HISTORY
 *      2006-Apr-26: Initial version by Darren Hart
 *      2006-May-08: Added atomic_{inc,set,get}, thread struct, debug function,
 *                      rt_init, buffered printing -- Vernon Mauery
 *      2006-May-09: improved command line argument handling
 *      2007-Jul-12: Added latency tracing functions -- Josh Triplett
 *      2007-Jul-26: Renamed to librttest.h -- Josh Triplett
 *      2009-Nov-4: TSC macros within another header -- Giuseppe Cavallaro
 *
 *****************************************************************************/

#ifndef LIBRTTEST_H
#define LIBRTTEST_H

//Kevin: attempt to solve redefinition issue
//#include </home/kevinpb/RTL/tests/librttest_version/src/include/rtai_types.h>
//
#include <sys/syscall.h>
#include <sys/timerfd.h>
#include <errno.h>
#include <getopt.h>
#include <math.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "list.h"
#include "config.h"

#define nsec_t 		RTIME
#define tsc_minus	rt_minus

#if KERN_RTAI
  #include <rtai_types.h>
  #include <rtai_lxrt.h>
  #include <rtai_sched.h>
  #include <rtai_sem.h>
#elif KERN_XENOMAI
  #include <native/task.h>
  #include <native/timer.h>
  #include <native/mutex.h>
  #include <native/sem.h>
  #include <native/cond.h>
  #include <rtdk.h>
  //Kevin: temporarily comment out these defines
  //#define printf	rt_printf
  //#define fprintf	rt_fprintf
#elif KERN_CHRONOS
  #include <chronos.h>
  /* ask antonio? */
  typedef unsigned long long RTIME;
#elif KERN_LITMUS
  #include <litmus.h>
  typedef lt_t RTIME;
#elif KERN_SCHED_DEAD
  typedef unsigned long long RTIME;
#else
  typedef unsigned long long RTIME;
#endif

#if !KERN_XENOMAI
    #include <semaphore.h>
#endif

#if KERN_SCHED_DEAD
    #include <linux/kernel.h>
    #include <linux/unistd.h>
    #include <linux/types.h>
    #include <time.h>
    
    #define SCHED_DEADLINE	6
    
        /* XXX use the proper syscall numbers */
    #ifdef __x86_64__
    #define __NR_sched_setparam2		313
    #define __NR_sched_getparam2		314
    #define __NR_sched_setscheduler2	315
    #endif
    
    #ifdef __i386__
    #define __NR_sched_setparam2		350
    #define __NR_sched_getparam2		351
    #define __NR_sched_setscheduler2	352
    #endif
    
    #ifdef __arm__
    #define __NR_sched_setscheduler2	378
    #define __NR_sched_setparam2		379
    #define __NR_sched_getparam2		380
    #endif
    
    #define SF_SIG_RORUN		2
    #define SF_SIG_DMISS		4
    #define SF_BWRECL_DL		8
    #define SF_BWRECL_RT		16
    #define SF_BWRECL_OTH		32
    
    #define RLIMIT_DLDLINE		16
    #define RLIMIT_DLRTIME		17

    struct sched_param2 {
        int sched_priority;
        unsigned int sched_flags;
        __u64 sched_runtime;
        __u64 sched_deadline;
        __u64 sched_period;

        __u64 __unused[12];
    };
#endif

extern void setup();
extern void cleanup();

extern int optind, opterr, optopt;
extern char *optarg;

#define _MAXTHREADS	256
#define CALIBRATE_LOOPS	1000000
int iters_per_us;

#define NS_PER_SEC	1000000000
#define NS_PER_MS	1000000
#define NS_PER_US	1000
#define US_PER_SEC	1000000
#define US_PER_MS	1000
#define MS_PER_SEC	1000

#if KERN_CHRONOS
    /* Which scheduler to use! */
    #define SET_SCHED_ALGO        SCHED_RT_GEDF

    /* Other necessary parameters */
    
    #define MAIN_PRIO             98
    #define TASK_RUN_PRIO         90
    #define SET_SCHED_FLAG        0
    #define SET_SCHED_PRIO        ((SET_SCHED_ALGO & SCHED_GLOBAL_MASK) ? TASK_RUN_PRIO : -1)
    #define SET_SCHED_PROC        0
#elif KERN_LITMUS
    #define CALL( exp ) do {  \
    int ret; \
    ret = exp; \ 
    if (ret != 0) \
        fprintf(stderr, "%s failed: %m\n", #exp); \
    else \
        fprintf(stderr, "%s ok.\n", #exp); \
    } while (0) 
#elif KERN_PREEMPT || KERN_IRMOS || KERN_SCHED_DEAD
    #define THREAD_PRIORITY 99
    #define PERIOD 100000
    #define DEADLINE 100000
    #define RUNTIME 100000
    #define NANO 1000000000 
#endif

#if KERN_SCHED_DEAD
    #define SCHED_DEADLINE 6
#endif

#if KERN_RTAI
	typedef RT_TASK* thr_t;
	typedef SEM mutex;
    typedef SEM sem_rt;
	typedef CND cond_t;
#elif KERN_XENOMAI
	typedef RT_TASK thr_t;
	typedef RT_MUTEX mutex;
	typedef RT_COND cond_t; //Kevin: removed pointer
    typedef RT_SEM sem_rt;
#elif KERN_CHRONOS
	typedef chronos_mutex_t mutex;
	typedef sem_t sem_rt;
	typedef pthread_t thr_t;
	typedef pthread_cond_t cond_t;
	typedef pthread_attr_t attr_t;
#else
	typedef pthread_t thr_t;
	typedef sem_t sem_rt;
	typedef pthread_mutex_t mutex;
	typedef pthread_attr_t attr_t;
	typedef pthread_cond_t cond_t;
#endif

//look into integrating into schedtest task
struct task_t {
#if KERN_LITMUS
    lt_t        exec_cost;
    lt_t        period;  
    lt_t        relative_deadline;    
    lt_t        phase;                                                                                                                                 
    unsigned int    cpu;
    unsigned int    priority;
    task_class_t    cls;
    budget_policy_t budget_policy; /* ignored by pfair */
#endif
	struct list_head list;
	thr_t desc;
    mutex lock;
#if !KERN_RTAI && !KERN_XENOMAI  //list of kernels that don't depend on pthread to spawn tasks
	attr_t attr;
#endif
	cond_t cond;
	void *arg;
	void *(*func)(void*);
	const char *name;	//!< Xenomai & RTAI (subsequently through nam2num)
	int prio;
	int policy;
	int flags;
	int stack_size;		//!< Xenomai & RTAI - To be deleted?
	int max_msg_size;	//!< RTAI
#if KERN_RTAI
	int cpus_allowed;	//!< RTAI
#else
	cpu_set_t cpus_allowed;	
#endif
	int id;

};

    static inline void dump_task_struct () {
        printf("dumping task_t: desc %d lock %d task %d\n",sizeof(thr_t),sizeof(mutex),sizeof(struct task_t)); 
    }


typedef struct task_t* task;
typedef struct task_t* task_rt;

typedef struct { volatile int counter; } atomic_t;

// flags for threads
#define THREAD_START 1
#define THREAD_QUIT  2
#define thread_quit(T) (((T)->flags) & THREAD_QUIT)

#define PRINT_BUFFER_SIZE (1024*1024*4)
#define ULL_MAX 18446744073709551615ULL // (1 << 64) - 1

extern mutex _buffer_mutex;
extern char * _print_buffer;
extern int _print_buffer_offset;
extern int _dbg_lvl;
extern double pass_criteria;

// 
/*--- Task Management Functions ---*/

/**
 * \brief Initialize the task set
 *
 * \param num Number of tasks in the task set
 * \return Pointer to the head of task set
 */
task taskset_init ( int num );

/**
 * \brief task_create: create a task managed trough scheduling policy, with priority prio running func as the thread function with arg as it's parameter.
 *
 * \param *t Pointer to the task parameters
 */
int task_create ( task t );

/**
 * \brief Start the execution for the i-th task.
 *
 * Only for Xenomai & RTAI systems (maybe... ;) )
 * \param id Number of the task to start
 */
int task_start ( int id );

/**
 * \brief Set the scheduling policy for the task passed as argument
 *
 * \param id Number of the task to which change policy
 */
int task_set_policy (int id, int policy);

/**
 * \brief Set the priority for the task passed as argument
 *
 * \param id Number of the task to which change priority
 */
int task_set_priority (int id, int prio);

/**
 * \brief Get one task
 *
 * \param id Number of the task to recover
 * \return Pointer to the task info structure
 */
task task_get ( int id );

/**
 * \brief Inspect the task structure
 *
 * \param t* Pointer to the task info structure to inspect
 */
void task_inspect ( task t );

/**
 * \brief Signal a single task to quit and then call join
 *
 * \param i the task to quit
 */
void task_join ( int i );

/**
 * \brief Wait for all threads to finish, calling task_quit_all internally
 */
void task_join_all();

/**
 * \brief Signal all threads to quit
 */
void task_quit_all();


/*--- Mutex Management Functions ---*/

/**
 * \brief Create a mutex
 */
int mutex_init ( mutex *m );

/**
 * \brief Destroy a mutex
 */
int mutex_destroy ( mutex *m );

/**
 * \brief Lock a mutex
 */
int mutex_lock ( mutex *m );

/**
 * \brief Unlock a mutex
 */
int mutex_unlock ( mutex *m );


/*--- Atomic functions ---*/

/**
 * \brief atomic_add - add integer to atomic variable and returns a value.
 * \param i: integer value to add
 * \param v: pointer of type atomic_t
 */
static inline int atomic_add(int i, atomic_t *v)
{
	/* XXX (garrcoop): only available in later versions of gcc */
#if HAVE___SYNC_ADD_AND_FETCH
	return __sync_add_and_fetch(&v->counter, i);
#else
	printf("%s: %s\n", __func__, strerror(ENOSYS));
	exit(1);
	return -1;
#endif
}

/**
 * \brief atomic_inc: atomically increment the integer passed by reference
 */
static inline int atomic_inc(atomic_t *v)
{
	return atomic_add(1, v);
}

/**
 * \brief atomic_get: atomically get the integer passed by reference
 */
static inline int atomic_get(atomic_t *v)
{
	return v->counter;
}

/**
 * \brief atomic_set: atomically get the integer passed by reference
 */
static inline void atomic_set(int i, atomic_t *v)
{
	v->counter = i;
}

//----------------------------------------------------------------------

/* buffer_init: initialize the buffered printing system
 */
void buffer_init();

/* buffer_print: prints the contents of the buffer
 */
void buffer_print();

/* buffer_fini: destroy the buffer
 */
void buffer_fini();

/* debug: do debug prints at level L (see DBG_* below).  If buffer_init
 * has been called previously, this will print to the internal memory
 * buffer rather than to stderr.
 * L: debug level (see below) This will print if L is lower than _dbg_lvl
 * A: format string (printf style)
 * B: args to format string (printf style)
 */
static volatile int _debug_count = 0;
#define debug(L,A,B...) do {\
	if ((L) <= _dbg_lvl) {\
		mutex_lock(&_buffer_mutex);\
		if (_print_buffer) {\
			if (PRINT_BUFFER_SIZE - _print_buffer_offset < 1000)\
				buffer_print();\
			_print_buffer_offset += snprintf(&_print_buffer[_print_buffer_offset],\
					PRINT_BUFFER_SIZE - _print_buffer_offset, "%06d: "A, _debug_count++, ##B);\
		} else {\
			fprintf(stderr, "%06d: "A, _debug_count++, ##B);\
		}\
		mutex_unlock(&_buffer_mutex);\
	}\
} while (0)
#define DBG_ERR  1
#define DBG_WARN 2
#define DBG_INFO 3
#define DBG_DEBUG 4

/* rt_help: print help for standard args */
void rt_help();

/* rt_init_long: initialize librttest
 * options: pass in an getopt style string of options -- e.g. "ab:cd::e:"
 *          if this or parse_arg is null, no option parsing will be done
 *          on behalf of the calling program (only internal args will be parsed)
 * longopts: a pointer to the first element of an array of struct option, the
 *           last entry must be set to all zeros.
 * parse_arg: a function that will get called when one of the above
 *            options is encountered on the command line.  It will be passed
 *            the option -- e.g. 'b' -- and the value.  Something like:
 *            // assume we passed "af:z::" to rt_init
 *            int parse_my_options(int option, char *value) {
 *                int handled = 1;
 *                switch (option) {
 *                    case 'a':
 *                        alphanum = 1;
 *                        break;
 *                    case 'f':
 *                    // we passed f: which means f has an argument
 *                        freedom = strcpy(value);
 *                        break;
 *                    case 'z':
 *                    // we passed z:: which means z has an optional argument
 *                        if (value)
 *                            zero_size = atoi(value);
 *                        else
 *                            zero_size++;
 *                    default:
 *                        handled = 0;
 *                }
 *                return handled;
 *            }           
 * argc: passed from main
 * argv: passed from main
 */
int rt_init_long(const char *options, const struct option *longopts,
		 int (*parse_arg)(int option, char *value),
		 int argc, char *argv[]);

/* rt_init: same as rt_init_long with no long options */
int rt_init(const char *options, int (*parse_arg)(int option, char *value),
	    int argc, char *argv[]);

/* return the delta in ts_delta
 * ts_end > ts_start
 * if ts_delta is not null, the difference will be returned in it
 */
void ts_minus(struct timespec *ts_end, struct timespec *ts_start, struct timespec *ts_delta);

/* return the sum in ts_sum
 * all arguments are not null
 */
void ts_plus(struct timespec *ts_a, struct timespec *ts_b, struct timespec *ts_sum);

/* put a ts into proper form (nsec < NS_PER_SEC)
 * ts must not be null
 */
void ts_normalize(struct timespec *ts);

/* convert nanoseconds to a timespec 
 * ts must not be null
 */
void nsec_to_ts(RTIME ns, struct timespec *ts);

/* convert a timespec to nanoseconds
 * ts must not be null
 */
int ts_to_nsec(struct timespec *ts, RTIME *ns);

/* return difference in microseconds */
RTIME rt_minus(RTIME start, RTIME end);

/* rt_gettime: get CLOCK_MONOTONIC time in nanoseconds
 */
RTIME rt_gettime();

// WORK & SLEEP FUNCTIONS
void rt_busywork(RTIME ns);

void rt_nanosleep(RTIME ns);

void rt_nanosleep_until(RTIME ns);

/* latency_trace_enable: Enable latency tracing via sysctls.
 */
void latency_trace_enable(void);

/* latency_trace_start: Start tracing latency; call before running test.
 */
void latency_trace_start(void);

/* latency_trace_stop: Stop tracing latency; call immediately after observing
 * excessive latency and stopping test.
 */
void latency_trace_stop(void);

/* latency_trace_print: Print latency trace information from
 * /proc/latency_trace.
 */
void latency_trace_print(void);

/** Kevin's Additions Start Here **/

/** TIMER_UTIL PRIMITIVES **/
inline void ts_zero(struct timespec* val);

/** PERIODIC PRIMITIVES **/
typedef struct __period_param {
	int timer;
	uint64_t missed_wakeups;
} period_param;

/* Function declarations for using the timer */
int start_periodic(struct timespec period, period_param* periodic_timer);
void wait_period(period_param* periodic_timer);
int end_periodic(period_param* periodic_timer);

#endif /* LIBRTTEST_H */

/** RT TASK PRIMITIVES **/
int rt_task_begin (RTIME exec_cost, RTIME period, period_param* my_period, int isPeriodic, int thread_id);
//int rt_task_begin (RTIME exec_cost, RTIME period, period_param* my_period) {

int rt_task_end (period_param* my_period, int isPeriodic);

/** RT JOB PRIMITIVES **/
long rt_job_begin (int prio, int max_util, struct timespec* deadline, struct timespec* period, unsigned long exec_time, period_param* periodic_timer, int isPeriodic);
//long rt_job_begin (int prio, int max_util, struct timespec* deadline, struct timespec* period, unsigned long exec_time, period_param* periodic_timer) {

long rt_job_end (int prio);

#if KERN_SCHED_DEAD
    int sched_setscheduler2(pid_t pid, int policy,
                  const struct sched_param2 *param);

    int sched_setparam2(pid_t pid,
                  const struct sched_param2 *param);

    int sched_getparam2(pid_t pid, struct sched_param2 *param);

    pid_t gettid();

#endif /* __DL_SYSCALLS__ */

/*--- Semaphore Management Functions ---*/

/**
 * \brief Create a semaphore
 */
int librt_sem_init ( sem_rt *s );

/**
 * \brief Destroy a sem
 */
int librt_sem_destroy ( sem_rt *s );

/**
 * \brief Lock a sem
 */
int librt_sem_wait ( sem_rt *s );

/**
 * \brief Unlock a sem
 */
int librt_sem_post ( sem_rt *s );

/**
 * \brief tryLock a sem
 */
int librt_sem_trywait ( sem_rt *s );

