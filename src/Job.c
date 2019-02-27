
#include <unistd.h>
#include "Job.h"

Job new_Job(){
    Job job;
    job.isBackground = false;
    job.processQueue = new_jobProcessQueue_t(0);
    for(int i = 0; i < 10; i++){
        pipe(job.pipes[i]);
    }
    return job;
}

void push_process_front_to_job(Job* job,pid_t pid){
    push_front_jobProcessQueue_t(&(job->processQueue),pid);
}

void destroy_job(Job* job){
    destroy_jobProcessQueue_t(&(job->processQueue));
    if(job->isBackground){
        free(job->cmd);
    }
}
void destroy_job_callback(Job job)
{
    destroy_jobProcessQueue_t(&(job.processQueue));
    if(job.isBackground){
        free(job.cmd);
    }
}
