#ifndef BACKGROUND_QUEUE_H
#define BACKGROUND_QUEUE_H

#include "deque.h"
#include "Job.h"

//Declare the queue data structure
IMPLEMENT_DEQUE_STRUCT (backgroundJobQueue_t, Job);

//Declare a queue for storing jobs
PROTOTYPE_DEQUE (backgroundJobQueue_t, Job);

#endif
