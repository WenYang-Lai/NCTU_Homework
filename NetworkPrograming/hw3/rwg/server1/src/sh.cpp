#include "sh.h"

bool SH::internal(char** cmd)
{ 
  if (strcmp(cmd[0],"setenv") == 0)
  {
    setenv(cmd[1], cmd[2], 1);
    for (vector<string>::iterator it = env.begin(); it!= env.end(); it++)
    {
      string str = *it;
      stringstream ss(str);
      string thisenv;
      getline(ss, thisenv, ',');
      if (strcmp(thisenv.c_str(), cmd[1]) == 0)
      {
        *it = string(cmd[1])+"="+string(cmd[2]);
        break;
      }
    }
    env.push_back(string(cmd[1])+"="+string(cmd[2]));
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

void SH::init(int id, CLIENT* cls, GLOBAL_PIPE* GP)
{
  m_count = 0;
  work = true;
  bool set = false;
  for (vector<string>::iterator it = env.begin(); it!= env.end(); it++)
  {
    string str = *it;
    stringstream ss(str);
    string thisenv;
    getline(ss, thisenv, ',');
    if (strcmp(thisenv.c_str(), "PATH") == 0)
    {
      *it = "PATH=bin:.";
      set = true;
      break;
    }
  }
  if (!set)
    env.push_back("PATH=bin:.");

  chdir("./rwg");
  clients = cls;
  client_id = id;
  global_pipe = GP;
}
void SH::clear_global_pipe()
{
  for (int i=0;i<128;i++)
  {
    if (global_pipe->is_used[i] && (global_pipe->src[i] == client_id || global_pipe->dest[i] == client_id))
    {
      close(global_pipe->pipe[i][0]);
      close(global_pipe->pipe[i][1]);
      global_pipe->is_used[i] = false;
    }
  }
}
bool SH::check_global_pipe(int id, int flag)
{
  for (int i=0;i<128;i++)
  {
    if (flag == 1)
    {
      if(!global_pipe->is_used[i])
      {
        pipe(global_pipe->pipe[i]);
        out_fd = global_pipe->pipe[i][1];
        global_pipe->src[i] = client_id;
        global_pipe->dest[i] = id;
        global_pipe->is_used[i] = true;
        return true;
      }
    }
    else
    {
      if (global_pipe->is_used[i] && global_pipe->src[i] == id && global_pipe->dest[i] == client_id)
      {
        global_pipe->is_used[i] = false;
        in_fd = global_pipe->pipe[i][0];
        return true;
      }
    }
  }
  return false;
}
void SH::prompt()
{
  input_redirection = false;
  output_redirection_fifo = false;
  output_redirection_file = false;
  // background = false;
  redirection_fail = false;
  empty_cmd = true;
  chatroom_cmd = false;
  delay_stderr = 0;
  delay_stdout = 0;

  multi_cmd.clear();
  pipe_num=0;
  printf("%% ");
  // getline(cin, cmd);
}

void SH::scan(string t_cmd)
{
  string tmp_cmd="";
  string tmp;
  cmd = t_cmd;
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
      if (tmp.length() == 1)
      {
        output_redirection_file = true;
        string file;
        iss >> file;
        if ( (out_fd = open(file.c_str(), O_WRONLY, 0)) == -1)
        {
          creat(file.c_str(), 0755);
          out_fd = open(file.c_str(), O_WRONLY, 0);
        }
      }
      else
      { 
        output_redirection_fifo = true;
        string str = tmp.substr(1);
        int target = atoi(str.c_str());
        sprintf(target_id, "%d", target);
        sprintf(self_id, "%d", client_id);
        /* check whether exist */
        for (int i=0;i<128;i++)
        {
          if (global_pipe->is_used[i] && global_pipe->src[i] == client_id && global_pipe->dest[i] == target)
          { 
            fprintf(stderr, "*** Error: the pipe #%d->#%d already exists. ***\n", client_id, target);
            redirection_fail = true;
            return;
          }
        }
        if (!clients[target].is_used)
        {
          fprintf(stderr, "*** Error: user #%d does not exist yet. ***\n", target);
          redirection_fail = true;
          return ;
        }
        if (!check_global_pipe(target, 1))
        {
          perror("can't get global pipe");
        }
      }
    }
    else if (tmp[0] == '<')
    {
      input_redirection = true;
      if (tmp.length() > 1)
      {
        string str = tmp.substr(1);
        int target = atoi(str.c_str());
        sprintf(target_id, "%d", target);
        sprintf(self_id, "%d", client_id);
        if (!check_global_pipe(target, 0))
        { 
          fprintf(stderr, "*** Error: the pipe #%d->#%d does not exist yet. ***\n", target, client_id);
          redirection_fail = true;
          return;
        }
        broadcast(("*** " + string(clients[client_id].name) + " (#" + string(self_id) + ") just received from " + clients[atoi(target_id)].name + " (#"  + string(target_id) + ") by '" + cmd + "' *** ").c_str());
        clear_buffer();
      }
      else
      {
        /*string file="";
        iss >> file;
        if ( (in_fd = open(file.c_str(), O_RDONLY, 0)) == -1 )
        {
          fprintf(stderr, "No such file %s exist!", file.c_str());
          redirection_fail = true;
          m_count++;
        }*/
        perror("not implemented input_redirect from file");
      }
    }
    else if(tmp == "yell" || tmp == "tell")
    { 
      multi_cmd.push_back(cmd);
      break;
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
  string check_str = str.substr(0,4);
  if (check_str == "yell")
  {
    string content = str.substr(5);
    char** args = new char* [2];
    args[0] = new char[5];
    strcpy(args[0], check_str.c_str());
    args[1] = new char [content.length()+1];
    strcpy(args[1], content.c_str());
    return args;
  }
  
  else if (check_str == "tell")
  {
    string tmp_str;
    char **args = new char* [3];
    args[0] = new char[5];
    strcpy(args[0], check_str.c_str());
    istringstream iss(str);
    iss >> tmp_str;
    tmp_str="";
    iss >> tmp_str;
    args[1] = new char[tmp_str.length()+1]; // id
    strcpy(args[1], tmp_str.c_str());
    tmp_str = str.substr(5+tmp_str.length()); //content
    args[2] = new char[tmp_str.length()+1];
    strcpy(args[2], tmp_str.c_str());
    return args;
  }

  else
  {
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
}
/* check whether is chatroom command */
bool SH::chatroom(char **cmd)
{
  if (strcmp(cmd[0], "tell") == 0)
  {
    int target = atoi(cmd[1]);
    string str(cmd[2]);
    if (str == "")
      perror("Unknown Command: tell");
    else
      send_message(target, ("*** " + string(clients[client_id].name) + " told you ***:  " + str).c_str());
    return true;
  }
  else if (strcmp(cmd[0], "yell") == 0)
  {
    string str(cmd[1]);
    if (str == "")
      perror("Unknown Command: yell");
    else
      broadcast(("*** " + string(clients[client_id].name) + " yelled ***:  " + str).c_str());
    return true;
  }
  else if (strcmp(cmd[0], "who") == 0)
  {
    who();
    return true;
  }
  else if (strcmp(cmd[0], "name") == 0)
  {
    string str="";
    for (int i=1; cmd[i] != NULL; i++)
    {
      if (i!=1)
        str = str + " ";
      str = str + cmd[i];
    }
    if (str == "")
      perror("Unknown Command: name");
    else
      name(str.c_str());
    return true;
  }
  return false;
}

bool SH::do_command()
{ 
  int child_pid;  
  int pip[pipe_num][2];
  char **first_cmd = parse_single_cmd(0); 
  if (!empty_cmd && !redirection_fail && !internal(first_cmd) && !chatroom(first_cmd))
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
          if(output_redirection_file == true || output_redirection_fifo == true)  
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
        if( execvp(single_cmd[0],single_cmd) < 0 )
        {
          fprintf(stderr ,"Unknown Command: [%s].\n", single_cmd[0]);
          exit(1);
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
        int status;
        wait(&status);
        //waitpid(-1, &status, WNOHANG);
        if (status != 0)
          break;
      }
    }
  }
  if (empty_cmd == false && chatroom_cmd == false)
  { 
    if (delay_pip.find(m_count) != delay_pip.end())
    {
      /*close(delay_pip[m_count].pip[0]);
      close(delay_pip[m_count].pip[1]);*/
      delay_pip.erase(m_count);
    }
    if (output_redirection_file)
      close(out_fd);
    /*  broadcast fifo */
    if (output_redirection_fifo == true || input_redirection == true)
    {
      if (!redirection_fail && output_redirection_fifo)
      { 
        close(out_fd);
        broadcast(("*** " + string(clients[client_id].name) + " (#" + string(self_id) + ") just piped '" + cmd + "' to " + clients[atoi(target_id)].name + " (#"  + string(target_id) + ") *** ").c_str());
      }
      if (!redirection_fail && input_redirection)
      { 
        close(in_fd);
        //broadcast(("*** " + string(clients[client_id].name) + " (#" + string(self_id) + ") just received from " + clients[atoi(target_id)].name + " (#"  + string(target_id) + ") by '" + cmd + "' *** ").c_str());
      }
    }
    /*for (int i=0;i<=pipe_num;i++)
      waitpid(-first_pid,0,0);*/
    m_count ++;
  }
  if (!work)
    return 0;
  return 1;
}

void SH::send_message(int target, const char *message)
{ 
  ////sem_wait(mutex);
  if (clients[target].is_used && clients[target].msg.m_count < 10)
    strcpy(clients[target].msg.messages[clients[target].msg.m_count++], message);
  else  
    fprintf(stderr, "*** Error: user #%d does not exist yet. ***\n",target);
  ////sem_signal(mutex);
}
void SH::broadcast(const char *message)
{ 
  //sem_wait(mutex);
  for (int i=1;i<=40;i++)
  {
    if (clients[i].is_used && clients[i].msg.m_count < 10)
    {
      strcpy(clients[i].msg.messages[clients[i].msg.m_count++], message);
    }
  }
  //sem_signal(mutex);
}

void SH::clear_buffer()
{
  //sem_wait(mutex);
  if (clients[client_id].msg.m_count > 0)
  { 
    //printf("\33[2K\r");
    do{
      cout << clients[client_id].msg.messages[--clients[client_id].msg.m_count] << endl;
    }while(clients[client_id].msg.m_count > 0);
    //prompt();
  }
  //sem_signal(mutex);
}

void SH::who()
{ 
  //sem_wait(mutex);
  printf("%-10s%-15s%-10s%28s\n", "<ID>","<nickname>" ,"<IP/port>" ,"<indicate me>");
  for (int i=1;i<=40;i++)
  {
    if (clients[i].is_used)
    {
      printf("%-10d%-15s%s/%d%15s\n", i, clients[i].name, inet_ntoa(clients[i].client_addr.sin_addr), clients[i].client_addr.sin_port, i== client_id ? "<-me" : "");
    }
      //cout << i+1 << "\t\t" <<  clients[i].name << "\t\t" << inet_ntoa(clients[i].client_addr.sin_addr) << '/' << clients[i].client_addr.sin_port << endl;
  }
  //sem_signal(mutex);
}

void SH::name(const char *name)
{
  for (int i=1;i<=40;i++)
  {
     if (clients[i].is_used && strcmp(clients[i].name, name) == 0)
     {
         broadcast(("*** User '(" + string(name)  + ")' already exists. ***").c_str());
         return;
     }  
  }                    
  strcpy(clients[client_id].name, name);
  char port[6];
  sprintf(port,"%d",clients[client_id].client_addr.sin_port);
  broadcast(("*** User from " + string(inet_ntoa(clients[client_id].client_addr.sin_addr)) + "/" + string(port) + " is named '" + name + "'. ***" ).c_str());
}

void SH::welcome()
{ 
  //check_online();
  puts("****************************************");
  puts("** Welcome to the information server. **");
  puts("****************************************");
  
  char port[6];
  sprintf(port,"%d",clients[client_id].client_addr.sin_port);
  broadcast(("*** User '(no name)' entered from "  + string(inet_ntoa(clients[client_id].client_addr.sin_addr)) + "/" + string(port) + ". ***").c_str());
}

void SH::check_online()
{ 
  char port[6];
  for (int i=1;i<=40;i++)
  { 
    if (clients[i].is_used )
    {
      sprintf(port,"%d",clients[i].client_addr.sin_port);
      puts(("*** User '(no name)' entered from "  + string(inet_ntoa(clients[i].client_addr.sin_addr)) + "/" + string(port) + ". ***").c_str()); 
    }
  }
}


void SH::Exit()
{
  broadcast(("*** User '" + string(clients[client_id].name) + "' left. ***").c_str());
  clients[client_id].is_used = false;
  clear_buffer();
  clear_global_pipe();
  //printf("\33[2K\r");
}
void SH::set_env(vector<string>& tmp_env)
{
  for (vector<string>::iterator it = tmp_env.begin(); it != tmp_env.end(); it++)
    env.push_back(*it); 
}

void SH::load_env()
{   
  for (vector<string>::iterator it = env.begin(); it != env.end(); it++)
  {
    string str = *it;
    stringstream ss(str);
    string thisenv, value;
    getline(ss , thisenv, '=');
    getline(ss , value, '=');
    setenv(thisenv.c_str(), value.c_str(), 1);
  }
}

void SH::start()
{
  /*prompt();
  do
  { 
    usleep(100000);
    clear_buffer();
    char str[65565];
    int len;
    len = read(STDIN_FILENO, str, 65565);
    if (len > 0)
    { 
      for (int i=0;i<65565;i++)
      {
        if (str[i] == '\n')
        { 
          str[i] = '\0';
          break;
        }
      }
      cmd = string(str);
      scan();
      do_command();
      prompt();
    }
    else if (len == 0)
    {
      
    }
    fflush(stdout);
    fflush(stderr);
  }while(work);
   */
}
