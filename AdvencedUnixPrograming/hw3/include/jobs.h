#ifndef __JOBS__
#define __JOBS__

#include <stdio.h>
#include <stdlib.h>
#include <map>

using namespace std;


struct Process{
  struct Process *next;
  char **argv;
  int pid;
  bool completed;
  bool stopped;

  Process(int i_pid, char** i_argv){
    next = NULL;
    pid = i_pid;
    argv = i_argv;
    completed = false;
    stopped = false;
  }
};

struct Job{

  Process *first_process;
  
  bool background;
  
  int pgid;
  int id;

  Job(int i_id, int i_pgid, Process* p, bool i_background) {
    background = i_background;
    id = i_id;
    pgid = i_pgid;
    first_process = p;
  }
};

extern map< int, Job* > job_list;


void make_job_continue(Job *j);
void make_job_stop(Job *j);
bool job_is_completed(Job *j);
Job* find_job(int pgid);
bool job_is_stopped (Job *j);
bool job_is_stopped (Job *j);
Process* job_find_process(Job *j, int pid);
void job_dump_process(Job *j);
#endif
