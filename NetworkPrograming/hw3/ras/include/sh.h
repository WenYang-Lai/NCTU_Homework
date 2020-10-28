#ifndef __SH__H__
#define __SH__H__

#include <stdio.h>
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
  void init();
  SH(){}
private:
  string cmd;
  int m_count;  // record how many line command excuted;
  int pipe_num;
  bool work;
  bool input_redirection_fail;
  bool empty_cmd;
  bool input_redirection;
  // bool background;
  bool output_redirection;
  int out_fd;
  int in_fd;
  int delay_stdout;
  int delay_stderr;

  vector<string> multi_cmd;
  set<int> process;
  map<int, PIPE> delay_pip;

  bool internal(char**);
  void prompt();
  void scan();
  bool do_command();
  
  char** parse_single_cmd(int);
};
#endif
