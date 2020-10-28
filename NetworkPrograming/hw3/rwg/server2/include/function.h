#ifndef FUNCTION__H__
#define FUNCTION__H__

#include <sys/sem.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>

#define MESSAGE_BUFFER 10
#define BIGCOUNT 1000

int sem_create(key_t , int);
void sem_rm(int);
void sem_signal(int);
void sem_wait(int);
void sem_op(int, int);
void sem_close(int);
struct CLIENT;
struct MSG;


struct MSG
{ 
  int m_count;
  char messages[10][1025];
};

struct CLIENT
{
  bool is_used;
  MSG msg;
  struct sockaddr_in client_addr;
  char name[1024];
  
};

struct FIFO_BUFFER{
  char path[128];
  int flag;
  int invoked_id[2];
  int fd;
  bool is_used;
};

#endif
