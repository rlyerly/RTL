/*
 * Common task and real-time utilities.
 * Author: Rob Lyerly <rlyerly@vt.edu>
 */

/*
 * Boolean values
 */
#define TRUE 1
#define FALSE 0

/*
 * Return values
 */
#define SUCCESS 0
#define FAILURE 1

/*
 * Task affinities
 */
#define AFFINITY 1
#define NO_AFFINITY 0

/*
 * Priority levels
 */
#define HIGH_PRIO 99
#define BUSY_PRIO 95
#define LOW_PRIO 90

/*
 * Clock that may be used by any application
 */
#define CLOCK CLOCK_MONOTONIC
