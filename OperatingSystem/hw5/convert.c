#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <sys/mman.h>

#define RECORD_SIZE 4096
char* addr;
char buffer[4100];
char valid[65536];
char tmp[5];

void reorder(int wfd, int high)
{ 
  tmp[4] = 0;
  for (int i=0;i<high;i++)
  {
    strncpy(tmp,(addr+i*RECORD_SIZE),4);
    write(wfd, tmp, 4);
  }
  for (int i=0;i<high;i++)
  {
    strncpy(buffer, (addr+i*RECORD_SIZE)+4, RECORD_SIZE-4);
    write(wfd, buffer, RECORD_SIZE-4);
  }
}

int main()
{
  struct stat sb;
  int dataset_fd = open("data.txt", O_RDONLY);
  int new_data_fd = open("new_data.txt", O_WRONLY | O_CREAT, 0666);
  
  fstat(dataset_fd,&sb);
  int N = sb.st_size; 
  memset(valid, '0', sizeof(valid));
  addr =  (char*)mmap(NULL, N , PROT_READ , MAP_PRIVATE ,dataset_fd , 0);
  printf("N : %d\n", N);
  reorder(new_data_fd, N/RECORD_SIZE);
  munmap(addr, N);

}
