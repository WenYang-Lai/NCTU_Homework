#include "jobs.h"

map< int, Job* > job_list;
 
void job_dump_process(Job *j){
  Process *p;
  for (p = j->first_process; p; p = p->next){
    char **argv = p->argv;
    while (*argv){
      fprintf(stderr, "%s ", *argv);
      if (p->next)
        fprintf(stderr, "| ");
      argv++;
    }
  }
      
  fprintf(stderr, "\n");
  return;

}

bool job_is_completed(Job *j){
  Process *p;
  for (p = j->first_process; p; p = p->next)
    if (!p->completed)
      return false;
  return true;
}

Job* find_job(int pgid){
  for (auto j: job_list)
    if (j.second->pgid == pgid)
      return j.second;
  return NULL; 
}


bool job_is_stopped (Job *j)
{
  Process *p;

  for (p = j->first_process; p; p = p->next)
    if (!p->completed && !p->stopped)
      return 0;
  return 1;
}

Process* job_find_process(Job *j, int pid){
  Process *p;

  for (p = j->first_process; p; p = p->next)
    if (p->pid == pid)
      return p;
  return NULL;

}
void make_job_continue(Job *j){
  Process *p;

  for (p = j->first_process; p; p = p->next)
    p->stopped = false;
}

void make_job_stop(Job* j){
  Process *p;
  
  fprintf(stderr, "hello\n");
  for (p = j->first_process; p; p = p->next)
    p->stopped = true;

}
