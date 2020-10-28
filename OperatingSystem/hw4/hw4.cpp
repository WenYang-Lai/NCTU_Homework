#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sys/time.h>
#include <string.h>
#include <queue>

using namespace std;
struct Job
{
  int start;
  int end;
  int level;
  Job(int s,int e, int l){start = s; end = e; level = l;}
  Job(){}
};

void quick_sort(int*, int, int);
void* thread_partition(void*);
void* dispatcher(void*);
int partition(int*, int, int);

int *copy_array;
int *active_array;
sem_t thread_pool, thread_buffer, queue_mutex; 
sem_t wake_up;

pthread_t *threads;
pthread_t master_thread;
int num_of_thread;
queue<Job> jobs;

int data_size;
int main(int argc, char* argv[])
{
  ifstream fin("input.txt");
  fin >> data_size;
  copy_array = new int[data_size];
  for (int i=0;i<data_size;i++)
    fin >> copy_array[i];
  sem_init(&thread_pool, 0, 0);
  sem_init(&queue_mutex, 0, 1);
  sem_init(&wake_up, 0, 0);
  for (num_of_thread=1; num_of_thread<=8;num_of_thread++)
  {  
    cout << "Thread : " << num_of_thread << endl;
    /* init thread, array and semaphore */
    sem_init(&thread_buffer, 0, num_of_thread); // number of threads
    active_array = new int[data_size];
    threads = new pthread_t[num_of_thread];
    bcopy(copy_array, active_array, sizeof(int)*data_size);
    /* timer */
    struct timeval start, end;
    int num, sec, usec;
    gettimeofday(&start, 0);
    /* create threads */
    pthread_create(&master_thread, NULL, dispatcher, NULL);
    for (int i=0;i<num_of_thread;i++)
    { 
      pthread_create(&(threads[i]), NULL ,thread_partition, 0);
    }
    /* do sort */
    jobs.push(Job(0, data_size, 0));
    sem_post(&thread_pool);
    pthread_join(master_thread, 0);
    /* end sort */
    gettimeofday(&end, 0);
    sec = end.tv_sec - start.tv_sec;
    usec = end.tv_usec - start.tv_usec;
    printf("Elapsed %lf ms.\n", (sec*1000+(usec/1000.0)));
    
    /* output result to file */
    char file_name[50];
    sprintf(file_name, "output_%d.txt", num_of_thread);
    ofstream fout(file_name);
    for (int i=0;i<data_size;i++)
    { 
      fout << active_array[i];
      if (i != data_size-1)
        fout << " ";
      else
        fout << endl;
    }
    delete [] active_array;
    delete [] threads;
    sem_destroy(&thread_buffer);
  } 
  sem_destroy(&wake_up);
  sem_destroy(&queue_mutex);
  sem_destroy(&thread_pool);
}
void* dispatcher(void *ptr)
{ 
  int remain_jobs = 15;
  while(remain_jobs > 0)
  {
    sem_wait(&thread_pool);
    sem_wait(&thread_buffer);
    for (int j=0;j<num_of_thread;j++)
    { 
        sem_wait(&queue_mutex);
        sem_post(&(wake_up));
        break;
    }
    remain_jobs--;
  }
  for(int i=0;i<num_of_thread;i++)
  { 
    sem_wait(&queue_mutex);
    jobs.push(Job(0,0,5));
    sem_post(&(wake_up));
  }
  for (int i=0;i<num_of_thread;i++)
    pthread_join(threads[i], 0);
  pthread_exit(0);
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
  int s, e, l;
  Job job;
  while(1)
  {
    sem_wait(&(wake_up));
    job = jobs.front();
    jobs.pop();
    sem_post(&queue_mutex);
    if (job.level == 5)
      pthread_exit(0);
    s = job.start;
    e = job.end;
    l = job.level;
    if (l<3)
    {
      if (s < e)
      { 
        int pivot = rand()%(e-s);
        swap(active_array[s], active_array[s+pivot]);
      }
      int pivot = s++;
      while (s < e)
      {
        if (active_array[s] <= active_array[pivot])
        {
          swap(active_array[s], active_array[pivot]);
          pivot = s++;
        }
        else if (active_array[s] > active_array[pivot])
          swap(active_array[s],active_array[--e]);      
      }
      jobs.push(Job(job.start, pivot, l+1));
      sem_post(&thread_pool);
      jobs.push(Job(pivot+1, job.end, l+1));
      sem_post(&thread_pool); 
    }
    else
      quick_sort(active_array, s, e);   
    sem_post(&thread_buffer);
  }
}
