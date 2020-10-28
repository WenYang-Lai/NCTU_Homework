#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <string.h>
#include <sys/wait.h>
#include <set>
#include <signal.h>
#include <fcntl.h>

using namespace std;
class SH
{
public:
  void start();
  SH(){}
private:
  string cmd;
  int pipe_num;
  bool scan_fail;
  bool background;
  bool input_redirection;
  bool output_redirection;
  bool empty_cmd;
  int out_fd;
  int in_fd;
  vector<string> multi_cmd;
  set<int> process;
  
  bool internal(char**);
  void init();
  void prompt();
  void scan();
  void do_command();
  
  char** parse_single_cmd(int);
};

bool SH::internal(char** cmd)
{
  /*if (strcmp(cmd[0],"kill") == 0)
  {
    
  }*/
  if (strcmp(cmd[0],"bg") == 0)
  {
    for (set<int>::iterator it = process.begin(); it != process.end(); it++ )
    {
      printf("pid : %d run in background\n", *it);
    }
    return true;
  }
  else
  {
    return false;
  }
}

void SH::init()
{
  input_redirection = false;
  output_redirection = false;
  background = false;
  scan_fail = false;
  empty_cmd = true;
  multi_cmd.clear();
  pipe_num=0;

}

void SH::prompt()
{
  printf("> ");
  getline(cin, cmd);
}

void SH::scan()
{
  string tmp_cmd="";
  
  for (int i=0; i< cmd.length(); i++)
  {
    if (cmd[i] == '|')
    {
      pipe_num++;
      multi_cmd.push_back(tmp_cmd);
      tmp_cmd="";
    }
    else if (cmd[i] == '&')
    {
      background = true;
    }
    else if (cmd[i] == '>')
    {
      output_redirection = true;
      string file="";
      for (i=i+1;i<cmd.length();i++)
      {
        file=file+cmd[i];
      }
      if ( (out_fd = open(file.c_str(), O_WRONLY, 0)) == -1)
      {
        creat(file.c_str(), 0755);
        out_fd = open(file.c_str(), O_WRONLY, 0);
      }
      
    }
    else if (cmd[i] == '<')
    {
      input_redirection = true;
      string file="";
      for (i=i+1;i<cmd.length();i++)
      {
        file=file+cmd[i];
      }
      if ( (in_fd = open(file.c_str(), O_RDONLY, 0)) == -1 )
      {
        printf("No such file %s exist!", file.c_str());
        scan_fail = true;
      }
    }
    else
    {
      tmp_cmd = tmp_cmd + cmd[i];
    }
  }
  multi_cmd.push_back(tmp_cmd);
  if (cmd.length())
    empty_cmd = false;
}

char** SH::parse_single_cmd(int i)
{
  string str = multi_cmd[i];
  istringstream iss(str);
  vector<string> tmp_argv;
  while(iss >> str)
    tmp_argv.push_back(str);
  char** args = new char* [tmp_argv.size()+1];
  for (int i=0;i<tmp_argv.size();i++)
  {
    args[i] = new char[tmp_argv[i].length()+1];
    strcpy(args[i], tmp_argv[i].c_str());
  }
  args[tmp_argv.size()] = NULL;
  
  return args;
}

void SH::do_command()
{ 
  int child_pid;
  int pip[pipe_num][2];
  if (empty_cmd == false)
  {
    for (int i=0;i<pipe_num;i++)
      pipe(pip[i]);
    
    for (int i=0;i<=pipe_num;i++)
    {
      if ( ( child_pid = fork() ) == -1)
      {
        printf("sys error!");
      }
      else if (child_pid == 0)
      { 
        if (i!=0)
          dup2(pip[i-1][0], STDIN_FILENO);
        if (i != pipe_num )
          dup2(pip[i][1], STDOUT_FILENO);
        if (i == 0 && input_redirection == true)
          dup2(in_fd, STDIN_FILENO);
        if (i == pipe_num && output_redirection == true)
          dup2(out_fd, STDOUT_FILENO);
        for (int j=0;j<pipe_num;j++)
        {
          close(pip[j][0]);
          close(pip[j][1]);
        }
        char **single_cmd = parse_single_cmd(i);
        if( execvp(single_cmd[0],single_cmd) < 0 )
        {
          printf("Command : %s not found.\n", single_cmd[0]);
          exit(1);
        }
      }
      else
      {
        process.insert(child_pid);
        int status;
        if (background == false)
        { 
          waitpid(child_pid, &status, 0);
          process.erase(child_pid);
        }
        if(i!=0)
          close(pip[i-1][0]);
        if (i!=pipe_num)
          close(pip[i][1]);
      }
    }
  }
}

void SH::start()
{ 
  while (true)
  {
    init();
    prompt();
    scan();
    if (scan_fail == false)
    {
      do_command();  
    }
  }
}

void sig_child(int signo)
{
  pid_t pid;
  int status;
  pid = waitpid(-1, &status, WNOHANG);
  if (pid!=-1 && (WIFEXITED(status) || WEXITSTATUS(status)))
  {
    printf("pid %d was terminated!\n", pid);
  }
  return;
}

int main(void)
{
  signal(SIGCHLD, &sig_child);
  SH shell;
  shell.start();
  
  return 0;
}
