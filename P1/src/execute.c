/**
 * @file execute.c
 *
 * @brief Implements interface functions between Quash and the environment and
 * functions that interpret an execute commands.
 *
 * @note As you add things to this file you may want to change the method signature
 */

#include "execute.h"

#include <stdio.h>
#include <fcntl.h>

#include "quash.h"
#include "Job.h"
#include "SingleJobQueue.h"
#include "BackgroundJobQueue.h"

// Remove this and all expansion calls to it
/**
 * @brief Note calls to any function that requires implementation
 */
#define IMPLEMENT_ME()                                                  \
  fprintf(stderr, "IMPLEMENT ME: %s(line %d): %s()\n", __FILE__, __LINE__, __FUNCTION__)

/***************************************************************************
 * Interface Functions
 ***************************************************************************/

job_id_t job_id = 1;

// Return a string containing the current working directory.
char* get_current_directory(bool* should_free) {
  *should_free = true;
  return(getcwd(NULL, 512));
}

// Returns the value of an environment variable env_var
const char* lookup_env(const char* env_var) {
  return(getenv(env_var));
}

// Check the status of background jobs
void check_jobs_bg_status() {
  if(is_empty_backgroundJobQueue_t(&backgroundQueue)){
      return;
  }

  int jobQueueLength = length_backgroundJobQueue_t(&backgroundQueue);

  for(int i = 0; i < jobQueueLength; i++){
      Job job;
      jobProcessQueue_t queue;
      int processQueue_length;
      bool job_still_has_running_process;;

      job = pop_front_backgroundJobQueue_t(&backgroundQueue);
      queue = job.processQueue;
      processQueue_length = length_jobProcessQueue_t(&queue);
      job_still_has_running_process = false;

      for(int j = 0; j < processQueue_length; j++){
          pid_t returnPid;
          int status;
          int pid;

          pid = pop_front_jobProcessQueue_t(&queue);
          returnPid = waitpid(pid, &status, WNOHANG);
        if (returnPid == -1){
            // error
        }
        else if (returnPid == 0){
            // child is still running
            job_still_has_running_process = true;
        }
        else if (returnPid == pid){
            // child is finished.
        }
        push_back_jobProcessQueue_t(&queue,pid);
      }

      if( job_still_has_running_process )
      {
        // re-add it to the queue
        push_back_backgroundJobQueue_t(&backgroundQueue, job);
      }
      else
      {
        // don't add it back, print message
        print_job_bg_complete(job.job_id, peek_front_jobProcessQueue_t(&job.processQueue), job.cmd);
        destroy_job(&job);
      }
  } //end for job_queue_length
}

// Prints the job id number, the process id of the first process belonging to
// the Job, and the command string associated with this job
void print_job(int job_id, pid_t pid, const char* cmd) {
  printf("[%d]\t%8d\t%s\n", job_id, pid, cmd);
  fflush(stdout);
}

// Prints a start up message for background processes
void print_job_bg_start(int job_id, pid_t pid, const char* cmd) {
  printf("Background job started: ");
  print_job(job_id, pid, cmd);
}

// Prints a completion message followed by the print job
void print_job_bg_complete(int job_id, pid_t pid, const char* cmd) {
  printf("Completed: \t");
  print_job(job_id, pid, cmd);
}

/***************************************************************************
 * Functions to process commands
 ***************************************************************************/
// Run a program reachable by the path environment variable, relative path, or
// absolute path
void run_generic(GenericCommand cmd) {
  // Execute a program with a list of arguments. The `args` array is a NULL
  // terminated (last string is always NULL) list of strings. The first element
  // in the array is the executable
  char* exec = cmd.args[0];
  char** args = cmd.args;

  execvp(exec, args);

  perror("ERROR: Failed to execute program");
}

// Print strings
void run_echo(EchoCommand cmd) {
  // Print an array of strings. The args array is a NULL terminated (last
  // string is always NULL) list of strings.
  char** str = cmd.args;

  for(int i = 0 ; NULL != str[i]; i++){
      printf("%s ", str[i]);
  }
  printf("\n");
  // Flush the buffer before returning
  fflush(stdout);
}

// Sets an environment variable
void run_export(ExportCommand cmd) {
  // Write an environment variable
  const char* env_var = cmd.env_var;
  const char* val = cmd.val;

  setenv(env_var, val, 1);
}

// Changes the current working directory
void run_cd(CDCommand cmd) {
  // Get the directory name
  const char* dir = cmd.dir;
  char* oldDir;
  char* newDir;


  // Check if the directory is valid
  if (dir == NULL) {
    perror("ERROR: Failed to resolve path");
    return;
  }

  oldDir = getcwd(NULL, 512);
  chdir(dir);
  newDir = getcwd(NULL, 512);
  setenv("PWD", newDir, 1);
  setenv("OLDPWD", oldDir, 1);

  //free directory strings
  free(newDir);
  free(oldDir);
}

// Sends a signal to all processes contained in a job
void run_kill(KillCommand cmd) {
  int signal = cmd.sig;
  job_id_t job_id = cmd.job;

  int jobQueueLength = length_backgroundJobQueue_t(&backgroundQueue);
  for(int i = 0; i < jobQueueLength; i++){
      Job job = pop_front_backgroundJobQueue_t(&backgroundQueue);
      if(job.job_id == job_id){
          jobProcessQueue_t queue;
          queue = job.processQueue;
          int processQueueLength = length_jobProcessQueue_t(&queue);

          for(int j = 0; j < processQueueLength; j++){
              int pid = pop_front_jobProcessQueue_t(&queue);
              kill(pid, signal);
              push_back_backgroundJobQueue_t(&backgroundQueue, job);
          }
      }
      else{
          push_back_backgroundJobQueue_t(&backgroundQueue, job);
      }
  }
}


// Prints the current working directory to stdout
void run_pwd() {
  bool should_free;
  char* currDirectorySTR = get_current_directory(&should_free);
  printf("%s\n", currDirectorySTR);

  if(should_free){
      free(currDirectorySTR);
  }
  // Flush the buffer before returning
  fflush(stdout);
}

// Prints all background jobs currently in the job list to stdout
void run_jobs() {
  //If queue is empty then task already complete
  if(is_empty_backgroundJobQueue_t(&backgroundQueue)){
      return;
  }

  int jobQueueLength = length_backgroundJobQueue_t(&backgroundQueue);

  for(int i = 0; i < jobQueueLength; i++){
      Job job = pop_front_backgroundJobQueue_t(&backgroundQueue);
      print_job(job.job_id, peek_front_jobProcessQueue_t(&job.processQueue), job.cmd);
      push_back_backgroundJobQueue_t(&backgroundQueue, job);
  }


  // Flush the buffer before returning
  fflush(stdout);
}

/***************************************************************************
 * Functions for command resolution and process setup
 ***************************************************************************/

/**
 * @brief A dispatch function to resolve the correct @a Command variant
 * function for child processes.
 *
 * This version of the function is tailored to commands that should be run in
 * the child process of a fork.
 *
 * @param cmd The Command to try to run
 *
 * @sa Command
 */
void child_run_command(Command cmd) {
  CommandType type = get_command_type(cmd);

  switch (type) {
  case GENERIC:
    run_generic(cmd.generic);
    break;

  case ECHO:
    run_echo(cmd.echo);
    break;

  case PWD:
    run_pwd();
    break;

  case JOBS:
    run_jobs();
    break;

  case EXPORT:
  case CD:
  case KILL:
  case EXIT:
  case EOC:
    break;

  default:
    fprintf(stderr, "Unknown command type: %d\n", type);
  }
}

/**
 * @brief A dispatch function to resolve the correct @a Command variant
 * function for the quash process.
 *
 * This version of the function is tailored to commands that should be run in
 * the parent process (quash).
 *
 * @param cmd The Command to try to run
 *
 * @sa Command
 */
void parent_run_command(Command cmd) {
  CommandType type = get_command_type(cmd);

  switch (type) {
  case EXPORT:
    run_export(cmd.export);
    break;

  case CD:
    run_cd(cmd.cd);
    break;

  case KILL:
    run_kill(cmd.kill);
    break;

  case GENERIC:
  case ECHO:
  case PWD:
  case JOBS:
  case EXIT:
  case EOC:
    break;

  default:
    fprintf(stderr, "Unknown command type: %d\n", type);
  }
}

/**
 * @brief Creates one new process centered around the @a Command in the @a
 * CommandHolder setting up redirects and pipes where needed
 *
 * @note Processes are not the same as jobs. A single job can have multiple
 * processes running under it. This function creates a process that is part of a
 * larger job.
 *
 * @note Not all commands should be run in the child process. A few need to
 * change the quash process in some way
 *
 * @param holder The CommandHolder to try to run
 *
 * @sa Command CommandHolder
 */
void create_process(CommandHolder holder, int pipeNum, Job* job) {
  // Read the flags field from the parser
  bool p_in  = holder.flags & PIPE_IN;
  bool p_out = holder.flags & PIPE_OUT;
  bool r_in  = holder.flags & REDIRECT_IN;
  bool r_out = holder.flags & REDIRECT_OUT;
  bool r_app = holder.flags & REDIRECT_APPEND; // This can only be true if r_out
                                               // is true

  pid_t pid = fork();
 //change so that parent is first
  if(pid == 0){
      if(r_in == true){
          int fileInput = open(holder.redirect_in, O_RDONLY);
          dup2(fileInput, STDIN_FILENO);
          close(fileInput);
      }
      if(r_out == true){
          FILE* file;
          if(r_app == true){
              file = fopen(holder.redirect_out, "a");
          }
          else{
              file = fopen(holder.redirect_out, "w");
          }
          dup2(fileno(file), STDOUT_FILENO);
      }
      if(p_out == true){
          dup2(job->pipes[pipeNum][1], STDOUT_FILENO);
          close(job->pipes[pipeNum][1]);
      }
      else{
          close(job->pipes[pipeNum][1]);
      }
      if(p_in == true){
          dup2(job->pipes[pipeNum - 1][0], STDIN_FILENO);
          close(job->pipes[pipeNum - 1][0]);
      }
      else if((pipeNum - 1) >= 0){
          close(job->pipes[pipeNum-1][0]);
      }

      child_run_command(holder.cmd);
      destroy_job(job);
      exit(EXIT_SUCCESS);
  }
  else{
      if(p_out == true){
          close(job->pipes[pipeNum][1]);
      }
      if(p_in == true){
          close(job->pipes[pipeNum-1][0]);
      }
      push_process_front_to_job(job, pid);
      parent_run_command(holder.cmd);
  }
}

void initBackgroundJobQueue(void)
{
  backgroundQueue = new_destructable_backgroundJobQueue_t(1,destroy_job_callback);
}

void destroyBackgroundJobQueue(void)
{
  destroy_backgroundJobQueue_t(&backgroundQueue);
}


// Run a list of commands
void run_script(CommandHolder* holders) {
  if (holders == NULL)
    return;

  check_jobs_bg_status();

  if (get_command_holder_type(holders[0]) == EXIT &&
      get_command_holder_type(holders[1]) == EOC) {
    end_main_loop();
    return;
  }

  CommandType type;
  Job job = new_Job();

  // Run all commands in the `holder` array
  for (int i = 0; (type = get_command_holder_type(holders[i])) != EOC; ++i)
    create_process(holders[i], i, &job);

  if (!(holders[0].flags & BACKGROUND)) {
    // Not a background Job
    // TODO: Wait for all processes under the job to complete
    while(!is_empty_jobProcessQueue_t(&job.processQueue)){
        int jobStatus;
        if(waitpid(peek_back_jobProcessQueue_t(&job.processQueue), &jobStatus, 0) != -1){
            pop_back_jobProcessQueue_t(&job.processQueue);
        }
    }
    destroy_job(&job);
  }
  else {
    // A background job.
    // TODO: Push the new job to the job queue
    job.isBackground = true;
    job.cmd = get_command_string();
    job.job_id = job_id++;
    push_back_backgroundJobQueue_t(&backgroundQueue, job);
    print_job_bg_start(job.job_id, peek_front_jobProcessQueue_t(&job.processQueue), job.cmd);
  }
}
