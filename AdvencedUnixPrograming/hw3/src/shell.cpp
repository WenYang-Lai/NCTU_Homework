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
#include <glob.h>
#include <map>
#include <algorithm>
#include <termios.h>
#include "jobs.h"
#include <errno.h>

   

    
using namespace std;

const char prompt_string[] = "shell-prompt$ ";
class SH
{
public:
  void start();
  static int fg_pgid;
  SH(){}
private:
  int shell_pgid;

  string cmd;
  int pipe_num;
  int origin_stdin, origin_stdout;
  bool scan_fail;
  bool background;
  bool input_redirection;
  bool input_redirection_fail;
  bool output_redirection;
  bool empty_cmd;
  int out_fd;
  int in_fd;
  vector<string> multi_cmd;
  
  bool internal(char**);
  void init();
  void prompt();
  void scan();
  void cmd_extension();
  void cmd_split_operators(string);

  int parse_pattern(int, string&);

  void check_glob(vector<string>&);
  vector<string> parse_glob(string&);
  
  void do_command();
  void do_internal(char **);
  
  void wait_for_job(Job*);

  void dup_origin();
  void dup_fd(int, int[][2]);

  char** parse_single_cmd(int);

};
int SH::fg_pgid = 0;

void sig_child(int signo)
{
  int pid;
  int status;
  Process* p;
  while ( (pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0 ){
    //fprintf(stderr, "handler, %d, %d\n", pid, getpid());
    //fflush(stderr);
    for (auto& j: job_list){
      if ( (p = job_find_process(j.second, pid)) != NULL){
        if (WIFSTOPPED(status))
          make_job_stop(j.second);
        else if ((WIFEXITED(status) || WEXITSTATUS(status) || WIFSIGNALED(status) || WCOREDUMP(status))){
          p->completed = true;
          if (job_is_completed(j.second)){
            if (j.second->background == true){
              fprintf(stderr, "\n[%d]\tdone\t", j.first);
              job_dump_process(j.second);
              fprintf(stderr, "%s", prompt_string);
              fflush(stderr);
            }
            job_list.erase(j.first);
          }
        }
      }
    }
  }
}

void sig_tstp(int signo){
  if (SH::fg_pgid != getpid()){
    for (auto& j: job_list){
      if (j.second->pgid == SH::fg_pgid)
        make_job_stop(j.second);
    }
    kill(-SH::fg_pgid, SIGTSTP); 
  }
}

void sig_int(int signo){
  if (SH::fg_pgid != getpid()){
    kill(-SH::fg_pgid, SIGINT); 
  }
}

void sig_quit(int signo){
  if (SH::fg_pgid != getpid()){
    kill(-SH::fg_pgid, SIGQUIT); 
  }
}

void SH::wait_for_job(Job* j){
  Process* p;
  while(!job_is_stopped(j) && job_list.find(j->id) != job_list.end()){
    int status;
    int pid = waitpid(-j->pgid, &status, WUNTRACED);
    //fprintf(stderr, "wait: %d, %d\n", pid, getpid());
    //fflush(stderr);
    if (pid > 0){
      p = job_find_process(j, pid);
      if (WIFSTOPPED(status))
        p->stopped = true;
      else if ((WIFEXITED(status) || WEXITSTATUS(status) || WIFSIGNALED(status) || WCOREDUMP(status)))
        p->completed = true;
    }
    usleep(100000);
  }
}

void SH::dup_origin(){
  dup2(origin_stdin, STDIN_FILENO); 
  dup2(origin_stdout, STDOUT_FILENO); 
}
 
void SH::dup_fd(int i, int pip[][2]){
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
}

void SH::do_internal(char** cmd){
  if (strcmp(cmd[0],"bg") == 0)
  {
    auto j = job_list.begin();
    j->second->background = true;
    if(j != job_list.end() && job_is_stopped(j->second))
      kill(-j->second->pgid, SIGCONT);
  }
  else if (strcmp(cmd[0], "unset") == 0){
    unsetenv(cmd[1]);
  }
  else if (strcmp(cmd[0], "export") == 0){
    string str = string(cmd[1]);
    int pos = str.find("=");
    if (pos == string::npos)
      return;
    string var = str.substr(0, pos);
    string val = str.substr(pos+1);
    setenv(var.c_str(), val.c_str(), 1);
  }
  else if (strcmp(cmd[0], "cd") == 0){
    chdir(cmd[1]);
  }
  else if (strcmp(cmd[0], "jobs") == 0){
    for (auto& j: job_list){
      fprintf(stderr, "[%d]\t%s\t", j.first, job_is_stopped(j.second) ? "Suspended" : "Continue");
      job_dump_process(j.second);
      fflush(stderr);
    }
  }
  else if (strcmp(cmd[0], "fg") == 0){
    for (auto& j: job_list){
      kill(-j.second->pgid, SIGCONT);
      make_job_continue(j.second);
      wait_for_job(j.second);
      fprintf(stderr, "[%d]\tDone\t", j.first);
      job_dump_process(j.second);
    }
    job_list.clear();
  }
  else if (strcmp(cmd[0], "exit") == 0)
    exit(0);
}

bool SH::internal(char** cmd)
{
  if (strcmp(cmd[0],"bg") == 0)
    return true;
  else if (strcmp(cmd[0], "fg") == 0)
    return true;
  else if (strcmp(cmd[0], "unset") == 0)
    return true;
  else if (strcmp(cmd[0], "export") == 0)
    return true;
  else if (strcmp(cmd[0], "cd") == 0)
    return true;
  else if (strcmp(cmd[0], "jobs") == 0)
    return true;
  else if (strcmp(cmd[0], "exit") == 0)
    return true;
  else
    return false;
}

void SH::init()
{
  shell_pgid = getpid();
  SH::fg_pgid = shell_pgid;
  
  origin_stdin = dup(STDIN_FILENO);
  origin_stdout = dup(STDOUT_FILENO);
  setpgid(getpid(), shell_pgid);
  tcsetpgrp(shell_pgid, shell_pgid);
  
  signal(SIGINT, &sig_int);
  signal(SIGQUIT, &sig_quit);
  signal(SIGTTIN, SIG_IGN);
  signal(SIGTTOU, SIG_IGN);
  signal(SIGCHLD, &sig_child);
  signal(SIGTSTP, &sig_tstp);

}

void SH::prompt()
{
  input_redirection = false;
  output_redirection = false;
  background = false;
  empty_cmd = true;
  input_redirection_fail = false;
  multi_cmd.clear();
  pipe_num=0;

  fprintf(stderr, "%s", prompt_string);
  fflush(stderr);
  if (!getline(cin, cmd)){
    exit(0);
  }
}

vector<string> SH::parse_glob(string& pattern)
{  
  vector<string> ret;
  glob_t glob_result;
  glob(pattern.c_str(), GLOB_TILDE|GLOB_NOSORT, NULL, &glob_result);
  for(int i=0; i < glob_result.gl_pathc; i++)
    ret.push_back(string(glob_result.gl_pathv[i]));
  globfree(&glob_result);
  return ret;
}

void SH::cmd_split_operators(string operators){
  for (int i=0; i<operators.length(); i++){
    int index = 0;
    while ( (index = cmd.find(operators[i], index)) != string::npos){
      cmd.insert(index+1, " ");
      cmd.insert(index, " ");
      index = index+2;
    }
  } 
}

void SH::cmd_extension(){
  string tmp_cmd = cmd;
  string str;
  string ans = "";
  istringstream iss(tmp_cmd);
  while (iss >> str){
    if ( str.find("*") != string::npos || str.find("?") != string::npos ){
      vector<string> ret = parse_glob(str);
      for (auto& s: ret)
        ans += (s + " ");
    }
    else
      ans += (str + " ");
  }
  cmd = ans;
}

int SH::parse_pattern(int i, string& pattern){
  int len = cmd.length();
  pattern = "";
  while(i < len && cmd[i] == ' ') i++;
  if (i == len) return i;
  else{
    while(i < len){
      if (cmd[i] == ' ' || cmd[i] == '>' || cmd[i] == '<' || cmd[i] == '|' || cmd[i] == '&')
        return i;
      else if (cmd[i] == '\\'){
        pattern += cmd[++i];
        i++;
      }
      else
        pattern += cmd[i++];
    }
  }
}

void SH::scan()
{
  string tmp_cmd="";
  string pattern="";
  int i=0; 
  while (i< cmd.length())
  {
    if (cmd[i] == '|')
    {
      if (tmp_cmd.length())
        multi_cmd.push_back(tmp_cmd);
      pipe_num++;
      tmp_cmd="";
      i++;
    }
    else if (cmd[i] == '&')
    {
      if (tmp_cmd.length())
        multi_cmd.push_back(tmp_cmd);
      background = true;
      i++;
    }
    else if (cmd[i] == '>')
    { 
      if (tmp_cmd.length())
        multi_cmd.push_back(tmp_cmd);
      output_redirection = true;
      string file="";
      i = parse_pattern(i+1, file);
      if ( (out_fd = open(file.c_str(), O_WRONLY, 0)) == -1)
      {
        creat(file.c_str(), 0755);
        out_fd = open(file.c_str(), O_WRONLY, 0);
      }
    }
    else if (cmd[i] == '<')
    {
      if (tmp_cmd.length())
        multi_cmd.push_back(tmp_cmd);
      input_redirection = true;
      string file="";
      i = parse_pattern(i+1, file);
      if ( (in_fd = open(file.c_str(), O_RDONLY, 0)) == -1 )
      {
        printf("No such file %s exist!\n", file.c_str());
        input_redirection_fail = true;
      }
    }
    else
    { 
      i = parse_pattern(i, pattern);
      tmp_cmd = tmp_cmd + pattern + " ";
    }
  }
  if (tmp_cmd.length())
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
  Process* p;
  Job* j;
  int j_id = 0;
  if (empty_cmd == false)
  { 
    if (input_redirection_fail)
      return;
    int first = 0;
    for (int i=0;i<pipe_num;i++)
      pipe(pip[i]);
    
    for (int i=0;i<=pipe_num;i++)
    {
      char **single_cmd = parse_single_cmd(i);
      if (internal(single_cmd)){
        dup_fd(i, pip);
        do_internal(single_cmd); 
        dup_origin();
      }
      else{
        if ( ( child_pid = fork() ) == -1) 
        { 
          printf("sys error!"); 
        } 
        else if (child_pid == 0) 
        { 
          dup_fd(i, pip);
          if (first == 0){
            tcsetpgrp(0, getpid());
            setpgid(getpid(), getpid());
          }
          else{
            tcsetpgrp(0, first);
            setpgid(getpid(), first);
          }
          if( execvp(single_cmd[0],single_cmd) < 0 ) 
          { 
            printf("Command : %s not found.\n", single_cmd[0]); 
            exit(1); 
          } 
        } 
        else 
        { 
          if (first == 0){ 
            SH::fg_pgid = child_pid; 
            first = child_pid; 

            while(job_list.find(++j_id) != job_list.end()); 
            p = new Process(child_pid, single_cmd); 
            j = new Job(j_id, child_pid, p, background); 
            job_list[j_id] = j; 
          } 
          else{ 
            p -> next = new Process(child_pid, single_cmd); 
            p = p -> next; 
          }
          if(i!=0) 
            close(pip[i-1][0]); 
          if (i!=pipe_num) 
            close(pip[i][1]); 
        }
      }
    }
    if (output_redirection)
      close(out_fd);
    if (input_redirection && !input_redirection_fail)
      close(in_fd);

    if (background == false && first != 0){
      wait_for_job(j);
      if (job_is_completed(j)){
        job_list.erase(j_id);
      }
    }
  }
  tcsetpgrp(0, shell_pgid);
  SH::fg_pgid = shell_pgid;
}

void SH::start()
{ 
  init();
  while (true)
  { 
    prompt();
    cmd_split_operators("<>|&");
    cmd_extension();
    scan();
    do_command();  
  }
}



int main(void)
{
  SH shell;
  shell.start();
  
  return 0;
}
