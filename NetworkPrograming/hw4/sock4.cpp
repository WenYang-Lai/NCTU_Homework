#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/wait.h>
#include <set>
#include <unistd.h>
#include <strings.h>
#include <sstream>
#include <string.h>
#include <fstream>

using namespace std;

set<int> clients;
int connect_server();

class SERVER
{
public:
  
  void init();
  void start();
private:   
  struct sockaddr_in server_addr, client_addr;
  int server_socket_fd;
  void transmit(int, int);
  void request_handler(int, sockaddr_in);
};

void SERVER::init()
{ 
  clients.clear();

  if ((server_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    puts("server: can't open stream socket");
  
  bzero((char*)&server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(5480);
  if(bind(server_socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    puts("server: can't bind local address");

  if( listen(server_socket_fd, 20) <0 )
    puts("Error: occur listen");


}

void SERVER::request_handler(int client_fd, sockaddr_in client_addr)
{
  char request[256];
  read(client_fd, request, sizeof(request));
  unsigned char VN = request[0];
  unsigned char CD = request[1];

  unsigned int DST_PORT = ((unsigned char)request[2]) << 8 | ((unsigned char)(request[3]));
  char DST_IP[17];
  sprintf(DST_IP, "%u.%u.%u.%u", (unsigned char)request[4], (unsigned char)request[5], (unsigned char)request[6], (unsigned char)request[7]);

  printf("VN=%u CD=%u\n", VN, CD);
  printf("<D_IP>\t:%s\n<D_PORT>\t:%d\n", DST_IP, DST_PORT);

  char response[256];
  strncpy(response, request, 8);
  response[0] = 0; response[1] = 91; 
 
    unsigned char client_ip[4];

    client_ip[3] = (client_addr.sin_addr.s_addr >> 24)%256; 
    client_ip[2] = (client_addr.sin_addr.s_addr >> 16)%256; 
    client_ip[1] = (client_addr.sin_addr.s_addr >> 8)%256; 
    client_ip[0] = (client_addr.sin_addr.s_addr)%256; 
    
    ifstream ifile("./client_sock4.conf");
    string str;
    bool access = false;
    while(getline(ifile, str))
    {
      istringstream iss(str);
      bool permit = true;
      for(int i=0;i<4;i++)
      { 
        string token;
        getline(iss, token, '.');
        if (token == "*")
          continue;
        else
        {
          if ((unsigned)atoi(token.c_str()) == (unsigned)(unsigned char)client_ip[i])
            continue;
          else
          { 
            permit = false;
            break;
          }
        }
      }
      if (permit)
      { 
        printf("<Reply>\t:Access client rule: %s\n",str.c_str());
        access = true;
        break;
      }
    }
    ifile.close();
    if (!access)
    {
      return;
    }


  ifile.open("./sock4.conf", fstream::in);
  while(getline(ifile, str))
  {
    istringstream iss(str);
    bool permit = true;
    for(int i=4;i<8;i++)
    { 
      string token;
      getline(iss, token, '.');
      if (token == "*")
        continue;
      else
      {
        if ((unsigned)atoi(token.c_str()) == (unsigned)(unsigned char)request[i])
          continue;
        else
        { 
          permit = false;
          break;
        }
      }
    }
    if (permit)
    { 
      printf("<Reply>\t:Access rule: %s\n",str.c_str());
      response[1] = 90;
      break;
    }
  }
  if (response[1] == 91)
  {
    fprintf(stderr, "<Reply>\t:No such rule accept.\n");
    exit(1);
  }
  int dst_fd;
  sockaddr_in dst_addr;
  if (CD == 1)
  { 
    fprintf(stderr, "<Command>\t:CONNECT\n");
    write(client_fd, response, 8); 
    dst_fd = socket(AF_INET, SOCK_STREAM, 0);
    dst_addr.sin_family = AF_INET;
    inet_aton(DST_IP, (in_addr*)&dst_addr.sin_addr.s_addr);
    dst_addr.sin_port = htons(DST_PORT);
    if(connect(dst_fd, (sockaddr*)&dst_addr, sizeof(dst_addr)) == -1)
    {
      fprintf(stderr, "Fail connect\n");
      exit(1);
    }
  }
  else if (CD == 2)
  { 
    fprintf(stderr, "<Command>\t:BIND\n");
    int bind_fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in bind_addr;
    bind_addr.sin_family = AF_INET;
    bind_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind_addr.sin_port = htons(INADDR_ANY);
    
    if (bind(bind_fd, (sockaddr*)&bind_addr, sizeof(bind_addr)) < 0)
    {
      printf("Bind Failed.\n");
    }

    struct sockaddr_in port;
    socklen_t len = sizeof(port);
    if (getsockname(bind_fd, (sockaddr *)&port, &len) < 0)
    {
      fprintf(stderr, "Get socket port error\n");
      exit(1);
    }
    if (listen(bind_fd, 5) < 0 )
    {
      fprintf(stderr, "Listen bind fd error.\n");
      exit(1);
    }
    /* port */
    response[2] = (unsigned char)(ntohs(port.sin_port)/256);
    response[3] = (unsigned char)(ntohs(port.sin_port)%256);
    /* ip */
    response[4] = response[5] = response[6] = response[7] = 0;
    write(client_fd, response, 8);   // tell to client let server know which port should connect to

    socklen_t dst_len = sizeof(dst_addr);
    dst_fd = accept(bind_fd, (sockaddr *)&dst_addr, &dst_len);
    close(bind_fd);
    write(client_fd, response, 8);   //after accept send reply one more 

  }
  else
  {
    fprintf(stderr, "No such CD: %d\n", CD);
    exit(1);
  }
  transmit(client_fd, dst_fd);
}

void SERVER::transmit(int client_fd, int dst_fd)
{
  int nfds = dst_fd > client_fd ? dst_fd +1 : client_fd +1;
  fd_set rfds, crfds;
  FD_ZERO(&rfds);
  FD_SET(client_fd, &rfds); FD_SET(dst_fd, &rfds);
  bool first = true;
  while(1)
  {
    crfds = rfds;
    select(nfds, &crfds, NULL, NULL, (struct timeval*)NULL);
    
    char buffer[65536];
    if (FD_ISSET(client_fd, &crfds))
    {
      int len = read(client_fd, buffer, sizeof(buffer));
      if (len == 0)
        exit(0);
      else if (len == -1)
      {
        fprintf(stderr, "read error.\n");
        exit(0);
      }
      else 
        write(dst_fd, buffer, len);
      
      if (first)
      {
        first = false;
        printf("Redirect Content: ");
        for (int j=0;j<max(len, 5);j++)
            printf("%d ", buffer[j]);
        printf("\n");
      }
    }
    if (FD_ISSET(dst_fd, &crfds))
    {
      int len = read(dst_fd, buffer, sizeof(buffer));
      if (len == 0)
        exit(0);
      else if (len == -1)
      {
        fprintf(stderr, "read error.\n");
        exit(0);
      }
      else
        write(client_fd, buffer, len);
      
      if (first)
      {
        first = false;
        printf("Redirect Content: ");
        for (int j=0;j<max(len, 5);j++)
            printf("%d ", buffer[j]);
        printf("\n");
      }
    }
  }
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
      signal(SIGINT, SIG_DFL);
      close(server_socket_fd);
      
      printf("<S_IP>\t:%s\n", inet_ntoa(client_addr.sin_addr));  //ip to string
      printf("<S_PORT>\t:%d\n", client_addr.sin_port);
      request_handler(socket_fd, client_addr);
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
  int pid, status;
  while( (pid = waitpid(-1, &status, WNOHANG)) > 0)
  {
     if (clients.find(pid) != clients.end())
        clients.erase(pid);
  }
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
