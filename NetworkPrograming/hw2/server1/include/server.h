#ifndef __SERVER__H__
#define __SERVER__H__

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/wait.h>
#include <set>
#include <unistd.h>
#include <strings.h>
#include <sys/select.h>
#include <vector>
#include "sh.h"

using namespace std;

set<int> clients;
vector<string> env;

class SERVER
{
public:
  
  void init();
  void start();
private:   
  struct sockaddr_in server_addr, client_addr;
  int server_socket_fd;
  fd_set rfds, wfds;
  fd_set rcfds, wcfds;
  int nfds;

  int origin_stdin, origin_stdout, origin_stderr;
  void flush();
  void dup_client_fd(int);
  void dup_recovery();
};

#endif
