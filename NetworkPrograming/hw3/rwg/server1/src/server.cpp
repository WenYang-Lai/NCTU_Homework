#include "server.h"

void SERVER::init()
{ 
  clients.clear();

  if ((server_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    puts("server: can't open stream socket");
  
  bzero((char*)&server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(5484);

  if(bind(server_socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    puts("server: can't bind local address");

  if( listen(server_socket_fd, 5) <0 )
    puts("Error: occur listen");
  
  FD_SET(server_socket_fd, &rfds);
  nfds = server_socket_fd + 1;

  origin_stdin = dup(0);
  origin_stdout = dup(1);
  origin_stderr = dup(2);
}


void SERVER::start()
{ 
  
  CLIENT clients[41];
  SH sh[41];
  GLOBAL_PIPE global_pipe;
  memset(&global_pipe, 0, sizeof(GLOBAL_PIPE));
  memset(clients, 0, sizeof(CLIENT)*40);
  int len = sizeof(client_addr);
  while (1)
  { 
    bcopy((char*)&rfds, (char*)&rcfds, sizeof(rcfds));
    if(select(nfds, &rcfds, (fd_set*)0, (fd_set*)0, (struct timeval*)0) < 0)
    {
      if(errno == EINTR)
        continue;
      perror("select error!");
      exit(1);
    }
    for (int fd=0; fd<nfds; fd++)
    {
      if (fd == server_socket_fd && FD_ISSET(fd,&rcfds))
      {
        int client_sockfd = accept(server_socket_fd, (struct sockaddr*)&client_addr, (socklen_t*)&len);
        FD_SET(client_sockfd, &rfds);
        if (client_sockfd >= nfds)
          nfds = client_sockfd +1;
        dup_client_fd(client_sockfd);
        for (int i=1;i<=40;i++)
        {
          if (!clients[i].is_used)
          { 
            clients[i].sockfd = client_sockfd;
            clients[i].client_addr = client_addr;
            strcpy(clients[i].name, "(no name)");
            sh[i].set_env(env);
            sh[i].load_env();
            sh[i].init(i, clients, &global_pipe);
            clients[i].is_used = true;
            sh[i].welcome();
            sh[i].clear_buffer();
            sh[i].prompt();
            flush();
            break;
          }
        }
        dup_recovery();
      }
      if (FD_ISSET(fd, &rcfds))
      { 
        dup_client_fd(fd);
        for (int i=1;i<=40;i++)
        {
          if (clients[i].sockfd == fd)
          {
            string str;
            getline(cin,str);
            sh[i].scan((str));
            sh[i].load_env();
            if (!sh[i].do_command())
            { 
              sh[i].Exit();
              sh[i].clear_buffer();
              flush();
              close(fd);
              dup_recovery();
              FD_CLR(fd, &rfds);
            }
            else
            { 
              sh[i].clear_buffer();
              sh[i].prompt();
              flush();
            }
          }
        }
      }
      for (int i=1;i<=40;i++)
      {
        if (clients[i].is_used)
        { 
          dup_client_fd(clients[i].sockfd);
          sh[i].clear_buffer();
          flush();
          dup_recovery();
        }
      }
    }
  }
}
void SERVER::dup_client_fd(int fd)
{
  dup2(fd, STDIN_FILENO); 
  dup2(fd, STDOUT_FILENO); 
  dup2(fd, STDERR_FILENO);
}
void SERVER::flush()
{
  fflush(stdout);
  fflush(stderr);
}

void SERVER::dup_recovery()
{
  dup2(origin_stdin, STDIN_FILENO);
  dup2(origin_stdout, STDOUT_FILENO);
  dup2(origin_stderr, STDERR_FILENO);
}

void SIGCHLD_handler(int sig)
{
    waitpid(-1, NULL, WNOHANG);
}
void SIGINT_handler(int sig)
{

}
int main(int argc, char** argv, char** envp)
{
  //signal(SIGCHLD, &SIGCHLD_handler);
  //signal(SIGINT, &SIGINT_handler);
  while(*envp)
  { 
    cout << *envp << endl;
    env.push_back(string(*envp++));
  }
  SERVER server;
  server.init();
  server.start();
}
