#include "function.h"

int sem_create(key_t key, int initval)
{
  int id, semval;
  struct sembuf op_lock[2] = {
    2, 0, 0,
    2, 1, SEM_UNDO
  };
  struct sembuf op_endcreate[2] = {
    1, -1, SEM_UNDO,
    2, -1, SEM_UNDO
  };
  if(key == IPC_PRIVATE) return(-1);
  else if(key == (key_t)-1) return (-1);

again:
  if((id = semget(key, 3, 0666 | IPC_CREAT)) < 0) return (-1);
  if(semop(id, &op_lock[0], 2) < 0)
  {
    if(errno == EINVAL) goto again;
    perror("can't lock");
  }
  if ((semval = semctl(id, 1, GETVAL, 0)) < 0) 
    perror("can't GETVAL");
  if(semval == 0)
  {
    if(semctl(id, 0, SETVAL, initval) < 0)
      perror("can't SETVAL[0]");
    if(semctl(id, 1, SETVAL, BIGCOUNT) < 0)
      perror("can't set bigcount");
  }
  if(semop(id, &op_endcreate[0], 2) < 0)
    perror("can't end create");

  return id;
}

void sem_rm(int id)
{
  if(semctl(id, 0, IPC_RMID, 0) < 0)
    perror("can't IPC_RMID");
}

void sem_close(int id)
{
  int semval;
  struct sembuf op_close[3] = {
    2, 0, 0,
    2, 1, SEM_UNDO,
    1, 1, SEM_UNDO
  };
  struct sembuf op_unlock[1] = {
    2, -1, SEM_UNDO
  };
  if(semop(id, &op_close[0], 3) < 0)
    perror("can't close");
  if(semval = semctl(id, 1, GETVAL, 0) < 0)
    perror("can't GETVAL");
  if(semval > BIGCOUNT) 
    perror("sem[1] > BIGCOUNT");
  else if (semval == BIGCOUNT)
    sem_rm(id);
  else
    if(semop(id, &op_unlock[0], 1) < 0)
      perror("can't unlock");
}
void sem_op(int id, int value)
{
  struct sembuf op_op[1] = {
    0, 99, SEM_UNDO
  };
  if((op_op[0].sem_op = value) == 0)
    perror("can't have value == 0");
  if(semop(id, &op_op[0], 1) < 0)
    perror("can't execute sem_op");
}
void sem_wait(int id)
{
  sem_op(id, -1);
}
void sem_signal(int id)
{
  sem_op(id, 1);
}
