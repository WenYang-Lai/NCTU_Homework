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
#include "function.h"


using namespace std;
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
  void init(int, int, CLIENT*, FIFO_BUFFER*);
  SH(){}
private:
  string cmd;
  //char fifopath[100];
  int m_count;  // record how many line command excuted;
  int pipe_num;
  bool work;
  bool redirection_fail;
  bool empty_cmd;
  bool chatroom_cmd;
  bool input_redirection;
  // bool background;
  bool output_redirection_file;
  bool output_redirection_fifo;
  
  int fifo_id;
  int out_fd;
  int in_fd;
  char self_id[4], target_id[4]; //use for cast client id to string;

  int delay_stdout;
  int delay_stderr;
  
  int mutex;
  CLIENT* clients;
  FIFO_BUFFER* fifo_buffer;
  int client_id;

  vector<string> multi_cmd;
  set<int> process;
  map<int, PIPE> delay_pip;
  
  void who();
  void name(const char*);
  void clear_buffer();
  void send_message(int, const char*);
  void broadcast(const char*);
  
  bool internal(char**);
  bool chatroom(char**);
  void check_online();

  void prompt();
  void scan();
  bool do_command();
  
  char** parse_single_cmd(int);

  bool check_fifo(const char*,int,int,int);
  void clear_fifo();
};
#endif
