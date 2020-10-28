#ifndef __SH__H__
#define __SH__H__

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <set>
#include <signal.h>
#include <fcntl.h>
#include <map>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>

using namespace std;

struct CLIENT;

struct GLOBAL_PIPE
{ 
  int pipe[128][2];
  int src[128];
  int dest[128];
  bool is_used[128];
};

struct PIPE
{
  int pip[2];
  PIPE(int t[2]){pip[0]=t[0]; pip[1]=t[1];}
  PIPE(){}
};
class SH
{
public:
  void start();
  void init(int, CLIENT*, GLOBAL_PIPE*);
  void set_env(vector<string>&);
  void load_env();
  SH(){}
  void prompt();
  void scan(string);
  void welcome();
  void Exit();
  bool do_command();
  
  void clear_buffer();
private:
  //char fifopath[100];
  int m_count;  // record how many line command excuted;
  int pipe_num;
  string cmd;
  bool work;
  bool redirection_fail;
  bool empty_cmd;
  bool chatroom_cmd;
  bool input_redirection;
  // bool background;
  bool output_redirection_file;
  bool output_redirection_fifo;
  
  int out_fd;
  int in_fd;

  char self_id[4], target_id[4]; //use for cast client id to string;

  int delay_stdout;
  int delay_stderr;
  
  int mutex;
  CLIENT* clients;
  GLOBAL_PIPE* global_pipe;
  int client_id;
  
  vector<string> env;
  vector<string> multi_cmd;
  set<int> process;
  map<int, PIPE> delay_pip;
  
  void who();
  void name(const char*);
  void send_message(int, const char*);
  void broadcast(const char*);
  
  void check_online();
  bool internal(char**);
  bool chatroom(char**);

  void clear_global_pipe();
  bool check_global_pipe(int, int); 
  char** parse_single_cmd(int);


};

struct MSG
{ 
  int m_count;
  char messages[10][1025];
};

struct CLIENT
{ 
  bool is_used;
  int sockfd;
  MSG msg;
  struct sockaddr_in client_addr;
  char name[1024];
  
};


#endif
