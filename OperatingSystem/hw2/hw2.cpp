#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <iostream>
#include <sys/time.h>
using namespace std;

void init(unsigned int *matrix_addr, int size)
{ 
  int count = 0;
  int it = 0;
  for (int j=0;j<size;j++)
    for (int k=0;k<size;k++)
      matrix_addr[count++] = (it++);
}
unsigned int element_cal(unsigned int* matrix_addr, int dimension, int i, int j)
{
  unsigned int result = 0;
  int x,y,count;
  for (x=i*dimension,y=j,count=0;count<dimension;count++,x++,y+=dimension)
      result += matrix_addr[x] * matrix_addr[y];
  return result;
}
void calculate(unsigned int* matrix_addr, int dimension, int p_num, int p_count)
{
  int start_i = (dimension*p_count)/p_num;
  int end_i = (dimension*(p_count+1))/p_num;
  int m_count = ( dimension * dimension ) + start_i*dimension;
  for (int i= start_i; i<end_i;i++)
    for (int j=0;j<dimension;j++)
      matrix_addr[m_count++] = element_cal(matrix_addr, dimension, i, j);
}
int main(void)
{
  int dimension;
  printf("Input the matrix dimension: ");
  scanf("%d", &dimension);
  int shmid = shmget(IPC_PRIVATE, sizeof(int) * dimension *dimension * 2, IPC_CREAT|0600);
  for (int i=0;i<16;i++)
  { 
    struct timeval start, end;
    gettimeofday(&start, 0);
    unsigned int* matrix_addr = (unsigned int*)shmat(shmid, NULL, 0);
    int first_pid = 0;
    unsigned int checksum = 0;
    init(matrix_addr, dimension); 
    for (int j=0;j<=i;j++)
    {
      int child_pid = fork();
      if (child_pid == 0)
      {
        matrix_addr = (unsigned int*)shmat(shmid, NULL, 0);
        calculate(matrix_addr,dimension,i+1,j);
        shmdt(matrix_addr);
        exit(0);
      }
      else
      {
        setpgid(child_pid, first_pid);
        if (!first_pid)
          first_pid = child_pid;
      }
    }
    for (int j=0;j<=i;j++)
      waitpid(-first_pid, NULL, 0);
    int offset = dimension*dimension;
    for (int x=0;x<dimension;x++)
      for (int y=0;y<dimension;y++)
        checksum += matrix_addr[offset++];
    shmdt(matrix_addr);
    gettimeofday(&end, 0);
    int sec = end.tv_sec - start.tv_sec;
    int usec = end.tv_usec - start.tv_usec;
    printf("Multiplying matrices using %d process\n", i+1);
    printf("Elapsed time: %lf sec, Checksum: %u\n", (sec*1000+(usec/1000.0))/1000, checksum);
  }
  shmctl(shmid, IPC_RMID, 0);
  return 0;
}
