#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sys/time.h>

using namespace std;

void quick_sort(int*, int, int);
void* thread_partition(void*);
int partition(int*, int, int);

int *MT_array;
int *ST_array;
int start_end[32]={0,0,0,0};
pthread_t thread[15];
int t_count[15] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
sem_t sem[24];


int main(int argc, char* argv[])
{
  //int array[] = {10,9,8,7,6,5,1,4,2,3};
  string str;
  printf("%s: ", "input file");
  cin >> str;
  ifstream fin(str);
  ofstream fout1("output1.txt");
  ofstream fout2("output2.txt");

  struct timeval start, end;
  int num, sec, usec;
  fin >> num;
  ST_array = new int[num];
  MT_array = new int[num];
  int tmp_num;
  for (int i=0;i<num;i++)
  {
    fin >> tmp_num;
    ST_array[i] = MT_array[i] = tmp_num;
  }
  start_end[3] = num;
  /* MT_sort */
  gettimeofday(&start, 0);
  for (int i=2;i<24;i++)
    sem_init(&(sem[i]), 0, 0);
  for (int i=0;i<15;i++)
    pthread_create(&(thread[i]), NULL, thread_partition, (void*)(t_count+i));
  for (int i=16; i<24;i++)
    sem_wait(&(sem[i]));
  
  gettimeofday(&end, 0);
  sec = end.tv_sec - start.tv_sec;
  usec = end.tv_usec - start.tv_usec;
  printf("multi-thread\nElapsed %lf ms.\n", (sec*1000+(usec/1000.0)));
 
  /*  ST_sort */
  gettimeofday(&start, 0);
  quick_sort(ST_array, 0, num);
  gettimeofday(&end, 0);
  sec = end.tv_sec - start.tv_sec;
  usec = end.tv_usec - start.tv_usec;
  printf("single-thread\nElapsed %lf ms.\n", (sec*1000+(usec/1000.0)));
   
  for (int i=0;i<num;i++)
  { 
    fout1 << MT_array[i];
    fout2 << ST_array[i];
    if (i != num-1)
    {
      fout1 << " ";
      fout2 << " ";
    }
    else
    {
      fout1 << endl;
      fout2 << endl;
    }
  }
 
}
void quick_sort(int* array, int s, int e)
{
  if(s<e)
  {
    int p = partition(array, s, e);
    if ((p) - s > 1)
      quick_sort(array, s, p);
    if (e - (p+1) > 1)
      quick_sort(array, p+1, e);
  }
}
int partition(int* array, int s, int e)
{ 
  int pivot = rand()%(e-s);
  swap(array[s], array[s+pivot]);
  pivot = s++;
  while (s < e)
  {
    if (array[s] <= array[pivot])
    {
      swap(array[s], array[pivot]);
      pivot = s++;
    }
    else if (array[s] > array[pivot])
      swap(array[s],array[--e]);      
  }
  return pivot;
}
void* thread_partition(void* ptr)
{ 
  int p = *((int*)ptr);
  if (p!=1)
    sem_wait(&(sem[p]));
  int s = start_end[p*2], e = start_end[p*2+1];
  if (p<=7)
  {

    if (s < e)
    { 
      int pivot = rand()%(e-s);
      swap(MT_array[s], MT_array[s+pivot]);
    }
    int pivot = s++;
    while (s < e)
    {
      if (MT_array[s] <= MT_array[pivot])
      {
        swap(MT_array[s], MT_array[pivot]);
        pivot = s++;
      }
      else if (MT_array[s] > MT_array[pivot])
        swap(MT_array[s],MT_array[--e]);      
    }
    start_end[p*2*2] = start_end[p*2];
    start_end[p*2*2+1] = pivot;
    start_end[(p*2+1)*2] = pivot +1;
    start_end[(p*2+1)*2+1] = start_end[p*2+1];
    
    sem_post(&(sem[p*2]));
    sem_post(&(sem[p*2+1]));
  }
  else
  {
    quick_sort(MT_array, s, e);  
    sem_post(&(sem[p+8]));
  }
  pthread_exit((void*)0);
}
