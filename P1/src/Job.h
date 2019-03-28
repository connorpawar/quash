#ifndef JOB_H
#define JOB_H

#include "SingleJobQueue.h"

typedef int job_id_t;

typedef struct Job
{
  jobProcessQueue_t processQueue; //carry pids of all processes with it
  int pipes[10][2];
  bool isBackground;
  job_id_t job_id;
  char* cmd;
} Job;

Job new_Job();

void push_process_front_to_job(Job* job, pid_t pid);

void destroy_job(Job* job);
void destroy_job_callback(Job job);

#endif
