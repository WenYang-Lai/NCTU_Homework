#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <map>
#include <sys/select.h>
#include <netdb.h>

#define PARSE_KEY 0
#define PARSE_VALUE 1

#define S_NOUSE 0
#define S_CONNECTING 1
#define S_DONE 2
#define S_PROXY 3

using namespace std;


struct CLIENT{
  struct sockaddr_in client_sin;
  int client_fd;
  FILE* file;
  int status;
  int send_message;
  char buffer[65536];
  int sent_num;
};

CLIENT clients[5];   // at most connect to 5 server
map<string, string> parameters; // store query parameter
fd_set rfds, crfds, wfds, cwfds;
int nfds = 0;
int connect_num = 0;


void parse_query_string(char *str);
void connect_server(int i);  
void response_basic_information();
void string_shift_right(char *str, int s, int& e);
void escaping_string(char *str);
int readline(int fd,char *ptr,int maxlen, int i);
