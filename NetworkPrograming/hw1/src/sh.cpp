#include "sh.h"

bool SH::internal(char** cmd)
{ 
  if (strcmp(cmd[0],"setenv") == 0)
  {
    setenv(cmd[1], cmd[2], 1);
    return true;
  }
  else if (strcmp(cmd[0], "printenv") == 0 )
  {
    printf("%s=%s\n", cmd[1], getenv(cmd[1]));
    return true;
  }
  else if (strcmp(cmd[0], "exit") == 0)
  {
    work = false;
    return true;
  }
  else
  {
    return false;
  }
}

void SH::init()
{
  m_count = 0;
  work = true;
  setenv("PATH", "bin:.", 1);
  chdir("./ras");
}

void SH::prompt()
{
  input_redirection = false;
  output_redirection = false;
  // background = false;
  input_redirection_fail = false;
  empty_cmd = true;
  delay_stderr = 0;
  delay_stdout = 0;
  multi_cmd.clear();
  pipe_num=0;
  printf("%% ");
  getline(cin, cmd);
}

void SH::scan()
{
  string tmp_cmd="";
  string tmp;
  istringstream iss(cmd);
  while (iss >> tmp)
  { 
    empty_cmd = false;
    if (tmp[0] == '|')
    {
      pipe_num++;
      if (tmp_cmd.length())
        multi_cmd.push_back(tmp_cmd);
      if (tmp.length()>1)
      {
        delay_stdout = atoi(tmp.substr(1).c_str());
        pipe_num--;
        if (delay_pip.find(m_count+delay_stdout) == delay_pip.end())
        { 
          int tmp_pipe[2];
          pipe(tmp_pipe);
          delay_pip[m_count+delay_stdout] = PIPE(tmp_pipe);
        }
      }
      tmp_cmd="";
    }
    else if (tmp[0] == '!')
    {
      if (tmp_cmd.length())
        multi_cmd.push_back(tmp_cmd);
      if (tmp.length()>1)
      {
        delay_stdout = delay_stderr = atoi(tmp.substr(1).c_str());
        if (delay_pip.find(m_count+delay_stderr) == delay_pip.end())
        {
          int tmp_pipe[2];
          pipe(tmp_pipe);
          delay_pip[m_count+delay_stderr] = PIPE(tmp_pipe);
        }   
      }
      tmp_cmd="";
    }
    else if (tmp[0] == '>')
    {
      output_redirection = true;
      string file;
      iss >> file;
      if ( (out_fd = open(file.c_str(), O_WRONLY, 0)) == -1)
      {
        creat(file.c_str(), 0755);
        out_fd = open(file.c_str(), O_WRONLY, 0);
      }
    }
    else if (tmp[0] == '<')
    {
      input_redirection = true;
      string file="";
      iss >> file;
      if ( (in_fd = open(file.c_str(), O_RDONLY, 0)) == -1 )
      {
        fprintf(stdout, "No such file %s exist!", file.c_str());
        input_redirection_fail = true;
        m_count++;
      }
    }
    else
    {
      tmp_cmd = tmp_cmd + tmp + " ";
    }
  }
  if (tmp_cmd.length())
    multi_cmd.push_back(tmp_cmd);
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

bool SH::do_command()
{ 
  int child_pid;  
  int pip[pipe_num][2];
  char **first_cmd = parse_single_cmd(0); 
  int first_pid = 0;
  if (!empty_cmd && !internal(first_cmd))
  {
    for (int i=0;i<=pipe_num;i++)
    { 
      if (i!=pipe_num)
        pipe(pip[i]);

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

        if (i == 0)
        {
          if (input_redirection == true)
            dup2(in_fd, STDIN_FILENO);
          else if(delay_pip.find(m_count) != delay_pip.end())
            dup2(delay_pip[m_count].pip[0], STDIN_FILENO);
        } 
        if (i == pipe_num)
        {
          if(output_redirection == true)  
            dup2(out_fd, STDOUT_FILENO);
          if(delay_stdout != 0)
            dup2(delay_pip[m_count+delay_stdout].pip[1], STDOUT_FILENO);
          if(delay_stderr !=0)
            dup2(delay_pip[m_count+delay_stderr].pip[1], STDERR_FILENO);
        }
        for (map<int, PIPE>::iterator it = delay_pip.begin(); it != delay_pip.end(); it++)
        {
          close(it->second.pip[0]);
          close(it->second.pip[1]);
        }
        if (i!=0)
          close(pip[i-1][0]);
        if (i!=pipe_num)
        {  
          close(pip[i][0]); close(pip[i][1]);
        }
        char **single_cmd = parse_single_cmd(i);
        if (!internal(single_cmd)) 
        { 
          if( execvp(single_cmd[0],single_cmd) < 0 )
          {
            printf("Unknown Command: [%s].\n", single_cmd[0]);
            exit(1);
          }
        }
      }
      else
      {
        /*setpgid(child_pid,first_pid);
        if (!first_pid)
          first_pid = child_pid;*/
        if(i!=0)
          close(pip[i-1][0]);
        if (i!=pipe_num)
          close(pip[i][1]);
        /* single fork single wait */
        if (i==0 && delay_pip.find(m_count) != delay_pip.end())
        {  
          close(delay_pip[m_count].pip[0]);
          close(delay_pip[m_count].pip[1]);
        }
        wait(0);
      }
    }
  }
  if (empty_cmd == false)
  { 
    if (delay_pip.find(m_count) != delay_pip.end())
    {
      /*close(delay_pip[m_count].pip[0]);
      close(delay_pip[m_count].pip[1]);*/
      delay_pip.erase(m_count);
    }
    /*for (int i=0;i<=pipe_num;i++)
      waitpid(-first_pid,0,0);*/
    m_count ++;
  }
  if (!work)
    return 0;
  return 1;
}

void SH::start()
{
  puts("****************************************");
  puts("** Welcome to the information server. **");
  puts("****************************************");
  
  init();
  do
  {
    prompt();
    scan();
  }while(do_command());
}
