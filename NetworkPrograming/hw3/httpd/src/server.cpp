#include "server.h"

void SERVER::init()
{ 
  clients.clear();

  if ((server_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    puts("server: can't open stream socket");
  
  bzero((char*)&server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(5487);

  if(bind(server_socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    puts("server: can't bind local address");

  if( listen(server_socket_fd, 0) <0 )
    puts("Error: occur listen");
  
}


void set_query_string(string& str)
{ 
  istringstream iss(str);
  /*  set REQUEST_METHOD */
  string tmp;
  iss >> tmp;
  setenv("REQUEST_METHOD", tmp.c_str(), 1);
  /* set SCRITP_NAME & QUERY_STRING */
  iss >> tmp;
  setenv("SCRIPT_NAME", tmp.c_str(), 1);
  int i=0, len = tmp.length();
  string value = "";
  while (i<len)
  {
    if (tmp[i]=='?')
    { 
      if (i+1 < tmp.length())
        value = tmp.substr(i+1);
      break;
    }
    i++;
  }
  setenv("QUERY_STRING", value.c_str(), 1);
  setenv("CONTENT_LENGTH", "0", 1);
  setenv("REMOTE_HOST", "self", 1);
  setenv("REMOTE_ADDR", "0.0.0.0", 1);
  setenv("AUTH_TYPE", "baka", 1);
  setenv("REMOTE_USER", "hentai", 1);
  setenv("REMOTE_IDENT", "...", 1);
}
string parse_path(string& str)
{ 
  int len = str.length();
  int start, count = 0;
  int i=0;
  while (str[i]!='/') i++;
  start = ++i;
  for(;i<len;i++)
  {
    if (str[i] == '#' || str[i] == ' ' || str[i] =='\t' || str[i] == '?')
      break;
    count++;
  }
  if (count == 0)
    return "";
  else
    return str.substr(start,count);
}

bool is_cgi(string& str)
{
  for (int i=1;i<str.length();i++)
  {
    if (i+3 < str.length())
    {
      if (str[i]=='.' && str[i+1]=='c' && str[i+2]=='g' && str[i+3]=='i')
        return true;
    }
    else
      return false;
  }
}

void SERVER::start()
{
  while (1)
  { 
    int len = sizeof(client_addr);
    int socket_fd = accept(server_socket_fd, (struct sockaddr*)&client_addr, (socklen_t*)&len);
    
    fflush(stdout);
    //string str;
    //while (getline(cin, str)) cout << str << endl;    
    int child_pid = fork();
    if (child_pid == 0)
    { 
      signal(SIGCHLD,SIG_DFL);
      dup2(socket_fd, STDIN_FILENO);
      dup2(socket_fd, STDOUT_FILENO);
      string request;
      getline(cin, request);
      if (is_cgi(request))
      {
        clearenv();
        set_query_string(request);
        string path = "cgi/" + parse_path(request);
        fprintf(stderr, "open : %s\n", path.c_str());
        if (access(path.c_str(), F_OK) < 0)
        { 
          printf("HTTP/1.1 404 Not Found\n");
          printf("Content-type: text/html\n\n");
          fprintf(stderr, "file : %s is not exist.\n", path.c_str());
        }
        else
        {
          printf("HTTP/1.1 200 OK\n");
          fflush(stdout);
          execl(path.c_str(),path.c_str(), NULL);
        }
      }
      else
      {
        string path = "public_html/" + parse_path(request);
        fprintf(stderr, "path : %s\n", path.c_str()); // config imformation
        ifstream iFile(path.c_str(), fstream::in);
        if (iFile.fail())
        {
          fprintf(stderr, "file : %s is not exist.\n", path.c_str());
          printf("HTTP/1.1 404 Not Found\n");
          printf("Content-type: text/html\n\n");
        }
        else
        { 
          printf("HTTP/1.1 200 OK\n");
          printf("Content-type: text/html\n\n");
          string str;
          while (iFile >> str)
            cout << str << endl;
        }
      }
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
