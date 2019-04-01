/** @file libscheduler.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libscheduler.h"
#include "../libpriqueue/libpriqueue.h"

float wait = 0.0;//time spent waiting
int numWait = 0;//number of times wait is called for avg
float response = 0.0;//time spent responding
int numResponse = 0;//number of times responded for avg
float turnAround = 0.0;//time spent turn around
int numTurnAround = 0;//number of time turn around is called
int numCores;//total number of cores
scheme_t appliedSchema;//current scheme scheduler is running with
priqueue_t* jobQueue;//priority queue for jobs
int currentTime;//the current time



/**
  Stores information making up a job to be scheduled including any statistics.

  You may need to define some global variables or a struct to store your job queue elements.
*/
typedef struct _job_t
{
	int m_jobNum;//number of the job
	int m_arrTime;//time the job arrived to the scheduler
	int m_runTime;//time job has spent running
	int m_priority;//priority of the job
	int m_remTime;//time remaining
	int m_firstTime;//first time scheduled
	int m_lastTime;//last time scheduled
} job_t;
job_t** jobs = NULL;

void updateTime(int time){
	currentTime = time;//update current time
	for(int i = 0; i<numCores; i++){
		if(jobs[i] != NULL){
			if(jobs[i]->m_firstTime==-1 && jobs[i]->m_lastTime != currentTime){
				jobs[i]->m_firstTime = jobs[i]->m_lastTime;
				response = jobs[i]->m_firstTime - jobs[i]->m_arrTime +response;
				numResponse ++;
			}
			jobs[i]->m_remTime = jobs[i]->m_remTime - (currentTime - jobs[i]->m_lastTime);
			jobs[i]->m_lastTime = currentTime;
		}
	}//loops through each of the jobs on each of the cores to update their times
}


int comparer(const void * job1, const void * job2){
	int result = 0;
	int timeDiff = (int)(((job_t*)job1)->m_arrTime - ((job_t*)job2)->m_arrTime);//calculate difference between job times
	if(appliedSchema == FCFS){
		return timeDiff;
	}//if first come first serve just return time difference
	if(appliedSchema == SJF){
		result = (int)(((job_t*)job1)->m_runTime - ((job_t*)job2)->m_runTime);//run time difference
		if(result == 0){
			return(timeDiff);//if there is no difference just return the time difference
		}
		else{
			return(result);
		}
	}//if shortest job first return differnce in runtime
	if(appliedSchema == PSJF){
		result = (int)(((job_t*)job1)->m_remTime - ((job_t*)job2)->m_remTime);//difference in remaining time
		if(result == 0){
			return(timeDiff);
		}
		else{
			return(result);
		}
	}//if priority shortest job first return remaining time
	if(appliedSchema == PRI || appliedSchema == PPRI){
		result = (int)(((job_t*)job1)->m_priority - ((job_t*)job2)->m_priority);//compare the priority
		if(result == 0){
			return(timeDiff);
		}
		else{
			return(result);
		}
	}//return difference of priority
	if(appliedSchema == RR){
		return(0);
	}//Round robin doesn't need to compare
	return(0);
}

job_t* jobRemove(int id, int jobNum){
	job_t* rem = jobs[id];
	rem->m_lastTime=-1;

	jobs[id] = NULL;
	priqueue_offer(jobQueue,rem);//remove job from queue
	return(rem);//return job that was removed
}

int coreAvail(){
	for(int i = 0; i<numCores; i++){
		if(jobs[i] == NULL){
			return(i);
		}//returns the index of the first core available or -1 if there isn't one
	}
	return(-1);
}

int preempt(job_t* ajob){
	int result = 0;
	int new_result = 0;
	int index = -1;
	for(int i = 0; i<numCores; i++){
		new_result = comparer(ajob, jobs[i]);//compare inputted job with job of each of the cores
		if(result>new_result){
			result = new_result;
			index = i;
		}
		else if(new_result == result){
			if(index!=-1){
				if(jobs[index]->m_arrTime < jobs[i]->m_arrTime){
					index = i;
				}
			}
		}
	}
	if(index>=0){
		jobRemove(index, jobs[index]->m_jobNum);
		jobs[index]=ajob;
		ajob->m_lastTime = currentTime;
	}
	return(index);
}

/**
  Initalizes the scheduler.

  Assumptions:
    - You may assume this will be the first scheduler function called.
    - You may assume this function will be called once once.
    - You may assume that cores is a positive, non-zero number.
    - You may assume that scheme is a valid scheduling scheme.

  @param cores the number of cores that is available by the scheduler. These cores will be known as core(id=0), core(id=1), ..., core(id=cores-1).
  @param scheme  the scheduling scheme that should be used. This value will be one of the six enum values of scheme_t
*/
void scheduler_start_up(int cores, scheme_t scheme)
{
	appliedSchema = scheme;//set global scheme var
	jobQueue = (priqueue_t*)malloc(sizeof(priqueue_t));//alloc space for queue
	priqueue_init(jobQueue,&comparer);//create priority queue
	numCores=cores;//set num Cores
	jobs = (job_t**)malloc(sizeof(job_t*)*numCores);//alloc space for jobs
	for(int i =0; i<numCores; i++){
		jobs[i]=NULL;
	}//Initialize all the jobs to null
	updateTime(0);//set time to 0 to start
}


/**
  Called when a new job arrives.

  If multiple cores are idle, the job should be assigned to the core with the
  lowest id.
  If the job arriving should be scheduled to run during the next
  time cycle, return the zero-based index of the core the job should be
  scheduled on. If another job is already running on the core specified,
  this will preempt the currently running job.
  Assumptions:
    - You may assume that every job wil have a unique arrival time.

  @param job_number a globally unique identification number of the job arriving.
  @param time the current time of the simulator.
  @param running_time the total number of time units this job will run before it will be finished.
  @param priority the priority of the job. (The lower the value, the higher the priority.)
  @return index of core job should be scheduled on
  @return -1 if no scheduling changes should be made.

 */
int scheduler_new_job(int job_number, int time, int running_time, int priority)
{
	updateTime(time);//update time to the one called with the new job
	job_t* newJob = (job_t*)malloc(sizeof(job_t));//alloc the space for the added job
	newJob->m_jobNum = job_number;
	newJob->m_arrTime = time;
	newJob->m_runTime = running_time;
	newJob->m_remTime = running_time;
	newJob->m_priority = priority;
	newJob->m_firstTime = -1;
	newJob->m_lastTime = -1;//initialize all the job variables
	int index = coreAvail();
	if(index!=-1){
		jobs[index]=newJob;
		newJob->m_lastTime = currentTime;
		return index;
	}
	else if(appliedSchema == PSJF || appliedSchema == PPRI){
		index = preempt(newJob);
		if(index==-1){
			priqueue_offer(jobQueue, newJob);
		}
		return index;
	}
	else{
		priqueue_offer(jobQueue, newJob);
		return(-1);
	}
}


/**
  Called when a job has completed execution.

  The core_id, job_number and time parameters are provided for convenience. You may be able to calculate the values with your own data structure.
  If any job should be scheduled to run on the core free'd up by the
  finished job, return the job_number of the job that should be scheduled to
  run on core core_id.

  @param core_id the zero-based index of the core where the job was located.
  @param job_number a globally unique identification number of the job.
  @param time the current time of the simulator.
  @return job_number of the job that should be scheduled to run on core core_id
  @return -1 if core should remain idle.
 */
int scheduler_job_finished(int core_id, int job_number, int time)
{
	updateTime(time);//update time
	job_t * jobFin = jobRemove(core_id,job_number);//remove finished job
	priqueue_remove(jobQueue,jobFin);//remove from queue

	wait = currentTime - jobFin->m_arrTime-jobFin->m_runTime + wait;//set wait time
	numWait++;
	turnAround = currentTime - jobFin->m_arrTime + turnAround;//set turn around time
	numTurnAround ++;

	free(jobFin);//free the job

	jobFin = priqueue_poll(jobQueue);
	if(jobFin){
		jobs[core_id]=jobFin;
		jobFin->m_lastTime = currentTime;
		return(jobFin->m_jobNum);
	}
	return -1;
}


/**
  When the scheme is set to RR, called when the quantum timer has expired
  on a core.

  If any job should be scheduled to run on the core free'd up by
  the quantum expiration, return the job_number of the job that should be
  scheduled to run on core core_id.

  @param core_id the zero-based index of the core where the quantum has expired.
  @param time the current time of the simulator.
  @return job_number of the job that should be scheduled on core cord_id
  @return -1 if core should remain idle
 */
int scheduler_quantum_expired(int core_id, int time)
{
	updateTime(time);
	jobRemove(core_id, jobs[core_id]->m_jobNum);//remove job

	job_t * jobExp = priqueue_poll(jobQueue);//poll job
	if(jobExp){
		jobs[core_id]=jobExp;
		jobExp->m_lastTime = currentTime;
		return(jobExp->m_jobNum);
	}
	return -1;
}


/**
  Returns the average waiting time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average waiting time of all jobs scheduled.
 */
float scheduler_average_waiting_time()
{
	if(numWait == 0){
		return(0.0);
	}
	else{
		return((float)wait/numWait);
	}
}


/**
  Returns the average turnaround time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average turnaround time of all jobs scheduled.
 */
float scheduler_average_turnaround_time()
{
	if(numWait == 0){
		return(0.0);
	}
	else{
		return((float)turnAround/numTurnAround);
	}
}


/**
  Returns the average response time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average response time of all jobs scheduled.
 */
float scheduler_average_response_time()
{
	if(numWait == 0){
		return(0.0);
	}
	else{
		return((float)response/numResponse);
	}
}


/**
  Free any memory associated with your scheduler.

  Assumptions:
    - This function will be the last function called in your library.
*/
void scheduler_clean_up()
{
	void * temp;
	while( (temp = priqueue_poll(jobQueue)) != NULL){
		free(temp);
	}//frees all of the jobs in the queue
	free(jobs);
	priqueue_destroy(jobQueue);
	free(jobQueue);
}


/**
  This function may print out any debugging information you choose. This
  function will be called by the simulator after every call the simulator
  makes to your scheduler.
  In our provided output, we have implemented this function to list the jobs in the order they are to be scheduled. Furthermore, we have also listed the current state of the job (either running on a given core or idle). For example, if we have a non-preemptive algorithm and job(id=4) has began running, job(id=2) arrives with a higher priority, and job(id=1) arrives with a lower priority, the output in our sample output will be:

    2(-1) 4(0) 1(-1)

  This function is not required and will not be graded. You may leave it
  blank if you do not find it useful.
 */
void scheduler_show_queue()
{

}
