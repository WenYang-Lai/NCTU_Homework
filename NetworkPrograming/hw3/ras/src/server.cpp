#include "server.h"
#include "sh.h"

void SERVER::init()
{ 
  clients.clear();

  if ((server_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    puts("server: can't open stream socket");
  
  bzero((char*)&server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(5483);

  if(bind(server_socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    puts("server: can't bind local address");

  if( listen(server_socket_fd, 5) <0 )
    puts("Error: occur listen");


}
void SERVER::start()
{
  while (1)
  { 
    int len = sizeof(client_addr);
    int socket_fd = accept(server_socket_fd, (struct sockaddr*)&client_addr, (socklen_t*)&len);
    int child_pid = fork();
    if (child_pid == 0)
    { 
      signal(SIGCHLD,SIG_DFL);
      close(server_socket_fd);
      dup2(socket_fd, STDIN_FILENO);
      dup2(socket_fd, STDOUT_FILENO);
      dup2(socket_fd, STDERR_FILENO);
       
      SH shell;
      shell.init();
      shell.start();
      close(socket_fd);
      exit(0);
    }
    else
    {
      clients.insert(child_pid);
      close(socket_fd);
    }
  }
}

void SIGCHLD_handler(int sig)
{
  int status;
  int pid;
  while (pid = waitpid(-1, &status, WNOHANG), pid != -1);
  clients.erase(pid);
}
void SIGINT_handler(int sig)
{
  for (set<int>::iterator it = clients.begin(); it!=clients.end();it++)
  {
    kill(*it, SIGINT);
    waitpid(*it, NULL, 0);
  }
  exit(0);
}
int main(void)
{
  signal(SIGCHLD, &SIGCHLD_handler);
  signal(SIGINT, &SIGINT_handler);
  SERVER server;
  server.init();
  server.start();
}
