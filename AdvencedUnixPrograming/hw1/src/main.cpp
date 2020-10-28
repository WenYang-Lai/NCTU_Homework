#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <regex.h>
#include <sys/types.h>

#define DEFAULT     0
#define TCP         1
#define UDP         2
#define TCP_UDP     3

#define BUFFER_SIZE 65536
using namespace std;

int flag = DEFAULT;

const char* short_opt = "tu";
const struct option long_opt[] = {
    {"tcp", 0, NULL, 't'},
    {"udp", 0, NULL, 'u'},
    {0, 0, 0, 0}
};

char buffer[BUFFER_SIZE];

regex_t filter_string;
const int n_match = 5;
regmatch_t match_array[n_match];

bool is_number(char*);
void parse_options(int argc, char** argv);
void parse_net(FILE* target_file, const char* type);
string parse_pid_argv(int inode);
string parse_address_and_port(char* token);



void parse_options(int argc, char** argv){
  
  char c;
  while ( (c = getopt_long(argc, argv, short_opt, long_opt, NULL)) != -1){
    switch(c)
    {
      case 't':
        if (flag == DEFAULT)
          flag = TCP;
        else
          flag |= TCP;
        break;
      case 'u':
        if (flag == DEFAULT)
          flag = UDP;
        else
          flag |= UDP; 
        break;
      default:
         fprintf(stderr, "Unknown argument!\n");
         fprintf(stderr, "./hw1 [--t|--tcp] [-u|--udp] [filter-string]\n");
         break;
    }
  }
  if (optind < argc)
    regcomp(&filter_string, argv[optind], REG_EXTENDED|REG_NOSUB);
  else
    regcomp(&filter_string, ".*", REG_EXTENDED|REG_NOSUB);
}

void parse_net(FILE* target_file, const char* type){
  char buffer[256];
  fgets(buffer, 256, target_file);
  
  string local_address_port, foreign_address_port, pid_argv;
  while (fgets(buffer, 256, target_file) != NULL){
    
    string data = string(type) + '\t';
    
    unsigned inode;
    char *token;

    token = strtok(buffer, " ");
    token = strtok(NULL, " ");
    local_address_port = parse_address_and_port(token); 
    data += local_address_port;

    token = strtok(NULL, " ");
    foreign_address_port = parse_address_and_port(token);
    data += foreign_address_port;

    for (int i=0;i<7;i++)
      token = strtok(NULL, " ");
    sscanf(token, "%d", &inode);
    
    pid_argv = parse_pid_argv(inode);
    data += pid_argv;
    
    if(!regexec(&filter_string, data.c_str(), n_match, match_array, REG_NOTBOL))
      printf("%-10s%-25s%-25s%s\n", type, local_address_port.c_str(), foreign_address_port.c_str(), pid_argv.c_str());
 
  }

}

string parse_address_and_port(char* token){
  unsigned port_i;
  char ip[256], port[32];
  sscanf(token, "%32[0-9A-F]:%X", ip, &port_i);

  if (strlen(ip) > 8){
    struct in6_addr addr;
    sscanf(ip, "%08X%08X%08X%08X", &addr.s6_addr32[0], &addr.s6_addr32[1], &addr.s6_addr32[2], &addr.s6_addr32[3]);
    inet_ntop(AF_INET6, &addr, ip, sizeof(ip));
  }
  else{
    struct in_addr addr;
    sscanf(ip, "%X", &addr);
    inet_ntop(AF_INET, &addr, ip, sizeof(ip));
  }
  if (port_i != 0)
    sprintf(port, "%d", port_i);  
  else
    sprintf(port, "%c", '*');
  return string(ip) + string(":") + string(port);
}


bool is_number(char* str){
  int i=0;
  while (str[i]!='\0'){
    if ( (str[i] >= '0' && str[i] <= '9' ) == false )
      return false;
    i++;
  }
  return true;
}

string parse_cmdline(char *cmd, int cmd_len){
  for (int i = 0; i < cmd_len; i++)
    if (cmd[i] == 0) cmd[i] = ' ';
  cmd[cmd_len] = 0;
  
  int cut_pos = 0;
  string short_cmd = string(cmd);
  for (int i=0;i<cmd_len;i++)
    if(short_cmd[i] == ' ')
      break;
    else if (short_cmd[i] == '/')
      cut_pos = i;
  return short_cmd.substr(cut_pos+1);
}

string parse_pid_argv(int target_inode){
  string pid_argv = "";
  
  DIR* proc_dir = opendir("/proc");
  struct dirent* ptr;
  while( ( ptr = readdir(proc_dir) ) != NULL ){
    if (is_number(ptr->d_name)){
      DIR* fd_dir = opendir( ("/proc/" + string(ptr->d_name) + "/fd").c_str() );
      struct dirent* fds;
      while( (fds = readdir(fd_dir)) != NULL){
        char linkname[256], cmd[1024];
        int link_length, inode;
        link_length = readlink( ("/proc/" + string(ptr->d_name) + "/fd/" + fds->d_name).c_str(), linkname, 256 );
        linkname[link_length] = 0;
        sscanf(linkname, "socket:[%d]", &inode);
        if (target_inode == inode){
          pid_argv += string(ptr->d_name) + "/";
          int cmd_file_fd = open( ("/proc/" + string(ptr->d_name) + "/cmdline").c_str(), O_RDONLY);
          int cmd_len = read(cmd_file_fd, cmd, 1024);
          closedir(proc_dir);
          closedir(fd_dir);
          return pid_argv + parse_cmdline(cmd, cmd_len);
        }
      }
      closedir(fd_dir);
    }
  }
}

void command_handler(char* type){
  
  printf("List of %s connections:\n", type);
  printf("%-10s%-25s%-25s%s\n", "Proto", "Local Address", "Foreign Address", "PID/Program name and arguments");
  
  char ipv4_file[256], ipv6_file[256];

  sprintf(ipv4_file, "/proc/net/%s", type);

  FILE* ipv4 = fopen(ipv4_file , "r");
  if(ipv4 != NULL){
    parse_net(ipv4, type); 
    fclose(ipv4);
  }
  sprintf(ipv6_file, "/proc/net/%s6", type);
  
  FILE* ipv6 = fopen(ipv6_file, "r");
  if(ipv6 != NULL){
    parse_net(ipv6, (string(type)+"6").c_str());
    fclose(ipv6);
  }
}

int main(int argc, char** argv)
{   
  parse_options(argc, argv );
  if (flag & TCP || flag == DEFAULT) command_handler("tcp");
  if (flag & UDP || flag == DEFAULT) command_handler("udp");
}
