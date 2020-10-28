#include "server.h"
#include "sh.h"

int mutex, shmid_client, shmid_fifo;

void SERVER::init()
{ 
  clients.clear();

  if ((server_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    puts("server: can't open stream socket");
  
  bzero((char*)&server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(5487);

  if(bind(server_socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    puts("server: can't bind local address");

  if( listen(server_socket_fd, 0) <0 )
    puts("Error: occur listen");

  if (( mutex = sem_create((key_t)5487, 1)) < 0)
    perror("can't create sem");
  
  if((shmid_client = shmget((key_t)5487, sizeof(CLIENT)*41, 0666|IPC_CREAT)) < 0)
  {
    perror("can't create shm1");
  }
  if ((shmid_fifo = shmget((key_t)5488, sizeof(FIFO_BUFFER)*128, 0666|IPC_CREAT)) < 0)
    perror("can't create shm2");

}
void SERVER::check_fifo_buffer(FIFO_BUFFER* fifo_buffer)
{
  for (int i=0;i<128;i++)
  {
    if (fifo_buffer[i].is_used && fifo_buffer[i].flag == 1)
    { 
      fifo_buffer[i].fd = open(fifo_buffer[i].path, O_RDONLY | O_NONBLOCK);
      if (fifo_buffer[i].fd > 0)
      { 
        cout << "access : " << fifo_buffer[i].path << endl;
        fifo_buffer[i].flag = 3;
      }
    }
    else if (fifo_buffer[i].is_used && fifo_buffer[i].flag == 0)
    {
      fifo_buffer[i].is_used = false;
      cout << "clear fd :" << fifo_buffer[i].fd << endl;
      close(fifo_buffer[i].fd);
    }
  } 
  fflush(stdout);
}
void SERVER::start()
{
  CLIENT *clients_msg = (CLIENT*)shmat(shmid_client, NULL, 0);
  FIFO_BUFFER *fifo_buffer = (FIFO_BUFFER*)shmat(shmid_fifo, NULL, 0);
  memset(clients_msg, 0, sizeof(CLIENT)*41); //set to zero
  memset(fifo_buffer, 0, sizeof(FIFO_BUFFER)*128);
  fcntl(server_socket_fd, F_SETFL, O_NONBLOCK);
  while (1)
  { 
    sem_wait(mutex);
    check_fifo_buffer(fifo_buffer);
    sem_signal(mutex);
    int id;
    int len = sizeof(client_addr);
    int socket_fd = accept(server_socket_fd, (struct sockaddr*)&client_addr, (socklen_t*)&len);
   
    /* modify shm */
    if (socket_fd > 0)
    {
      cout << "accept client" << endl;
      sem_wait(mutex);
      cout << "access sem" << endl;
      for (int i=1;i<=40;i++)
      {
        if (!clients_msg[i].is_used)
        {
          id = i;
          strcpy(clients_msg[i].name, "(no name)");
          clients_msg[i].is_used = true;
          clients_msg[i].msg.m_count = 0;
          clients_msg[i].client_addr = client_addr;
          cout << "connet clients to id: " << i << endl;
          break;
        }
      }
      sem_signal(mutex);
      /* end critical section */
      int child_pid = fork();
      if (child_pid == 0)
      { 

        signal(SIGCHLD,SIG_DFL);
        fcntl(socket_fd, F_SETFL, O_NONBLOCK);
        close(server_socket_fd);
        dup2(socket_fd, STDIN_FILENO);
        dup2(socket_fd, STDOUT_FILENO);
        dup2(socket_fd, STDERR_FILENO);
         
        SH shell;
        shell.init(id, mutex, clients_msg, fifo_buffer);
        shell.start();
        sem_wait(mutex);
        clients_msg[id].is_used = false;
        sem_signal(mutex);

        //close(socket_fd);
        //shmdt(clients_msg);


        exit(0);
      }
      else
      {
        clients.insert(child_pid);
        close(socket_fd);
      }
    }
    usleep(10000);
  }
}

void SIGCHLD_handler(int sig)
{
  int pid;
  pid = waitpid(-1, 0, 0);
  cout << "pid :" << pid << " terminated!"<< endl;
  clients.erase(pid);
}
void SIGINT_handler(int sig)
{
  for (set<int>::iterator it = clients.begin(); it!=clients.end();it++)
  {
    kill(*it, SIGINT);
    waitpid(*it, NULL, 0);
  }
  shmctl(shmid_client, IPC_RMID, 0);
  shmctl(shmid_fifo, IPC_RMID, 0);
  semctl(mutex, IPC_RMID, 0);
  exit(0);
}
int main(void)
{
  signal(SIGCHLD, &SIGCHLD_handler);
  signal(SIGINT, &SIGINT_handler);
  SERVER server;
  server.init();
  server.start();
}
