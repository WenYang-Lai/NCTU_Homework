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

using namespace std;

set<int> clients;

class SERVER
{
public:
  
  void init();
  void start();
private:   
  struct sockaddr_in server_addr, client_addr;
  int server_socket_fd;
};
#endif
