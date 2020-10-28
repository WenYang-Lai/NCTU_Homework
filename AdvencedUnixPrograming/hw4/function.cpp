#include "function.h"

int readline(int fd, char* buffer, size_t size){
  char ch;
  int offset = 0;
  int len;
  while (1){
    len = read(fd, &ch, 1);
    if (len == 0)
      return 0;
    else if (len == -1){
      if (errno == EINTR)
        continue; 
      else if (errno != EAGAIN){
        fprintf(stderr, "Error read\n");
        exit(1);
      }
      else if (offset == 0)
        return -1;
      else
        continue;
    }
    else{
      buffer[offset++] = ch;
      if (ch == '\n'){
        buffer[offset] = '\0'; 
        return offset;
      }
    }
  }
}

void writen(int fd, char* buffer, size_t size){
  int len;
  while (size > 0){
    len = write(fd, buffer, size);
    if (len == -1){
      if (errno == EINTR)
        continue;
    }
    buffer += len;
    size -= len;
  }
}

