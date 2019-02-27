#ifndef QUEUE_H
#define QUEUE_H

#include "deque.h"


/*
 * Declare the queue
 */
IMPLEMENT_DEQUE_STRUCT (jobProcessQueue_t, pid_t);

/*
 * Create a queue for storing pids
 */
PROTOTYPE_DEQUE (jobProcessQueue_t, pid_t);

#endif
