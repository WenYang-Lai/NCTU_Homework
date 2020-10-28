#include "othello.h"
#include "function.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <strings.h>
#include <sys/select.h>
#include <fcntl.h>
#include <getopt.h>
#include <netdb.h>
#include <pthread.h>
#include <set>
#include <queue>

#define TERMINATE -2
#define STOP      0
#define RESET     -3

using namespace std;
set< pair<int, int> > valid_set;

static int width;
static int height;
static int cx = 3;
static int cy = 3;

static pthread_t thread;
static pthread_mutex_t thread_mutex = PTHREAD_MUTEX_INITIALIZER;

int self_id, opponent_id;
int socket_fd;

int current = 0;

struct sockaddr_in server_addr, client_addr;

static const char* short_opt = "s:c:";
static const struct option long_opt[] = {
    {"server", 1, NULL, 's'},
    {"client", 1, NULL, 'c'},
    {0, 0, 0, 0}
};

static char buffer[1024];

void launch_server(int port){
  
  int server_socket_fd;
  if ((server_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    puts("server: can't open stream socket");
  
  bzero((char*)&server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);
  
  if(bind(server_socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    puts("server: can't bind local address");

  if( listen(server_socket_fd, 20) <0 )
    puts("Error: occur listen");
  
  fprintf(stderr, "Waiting for client connect...");

  int len = sizeof(client_addr);
  socket_fd = accept(server_socket_fd, (struct sockaddr*)&client_addr, (socklen_t*)&len);
  
  fcntl(socket_fd, F_SETFL, O_NONBLOCK);
  close(server_socket_fd);
}

void connect_server(char* host, int port){
  socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr = *((struct in_addr *)(gethostbyname(host)->h_addr));
  server_addr.sin_port = htons(port);
  
  if(connect(socket_fd, (sockaddr*)&server_addr, sizeof(server_addr)) == -1)
  {
    fprintf(stderr, "Fail connect\n");
    exit(1);
  }
  fcntl(socket_fd, F_SETFL, O_NONBLOCK);
}

void parse_options(int argc, char** argv){
  char c;
  if ( (c = getopt_long(argc, argv, short_opt, long_opt, NULL)) == -1)
    fprintf(stderr, "Need option with -s or -c\n");
  else{
    current = PLAYER1;
    if (c == 's'){
      launch_server(atoi(optarg));
      self_id = PLAYER1;
      opponent_id = PLAYER2;
    }
    else if (c == 'c'){
      self_id = PLAYER2;
      opponent_id = PLAYER1;
      char hostname[256];
      char *ptr = strtok(optarg, ":");
      strcpy(hostname, ptr);
      ptr = strtok(NULL, "");
      connect_server(hostname, atoi(ptr));
    }
    else{
      fprintf(stderr, "Unknown argument\n");
      exit(1);
    }
  }
}

void check_rule(int x, int y){
  pair<int, int> pos(x,y);
  queue< pair<int, int> > q;
  q.push(pos);
  while (!q.empty()){
     pos = q.front();
     q.pop();
     x = pos.first; y = pos.second;
     if (x - 2 >= 0 && board[y][x-1] != board[y][x] && board[y][x] == board[y][x-2] && board[y][x-1] !=0){
       board[y][x-1] = board[y][x]; q.push(pair<int,int>(x-1, y)); draw_cursor(x-1, y, 1);
     }
     if (y - 2 >= 0 && board[y-1][x] != board[y][x] && board[y][x] == board[y-2][x] && board[y-1][x] !=0){
       board[y-1][x] = board[y][x]; q.push(pair<int,int>(x, y-1)); draw_cursor(x, y-1, 1);
     }
     if (x + 2 < 8 && board[y][x+1] != board[y][x] && board[y][x] == board[y][x+2] && board[y][x+1] !=0){
       board[y][x+1] = board[y][x]; q.push(pair<int,int>(x+1, y)); draw_cursor(x+1, y, 1);
     }
     if (y + 2 < 8 && board[y+1][x] != board[y][x] && board[y][x] == board[y+2][x] && board[y+1][x] !=0){
       board[y+1][x] = board[y][x]; q.push(pair<int,int>(x, y+1)); draw_cursor(x, y+1, 1);
     }
     if (y - 2 >= 0 && x - 2 >= 0 && board[y-1][x-1] != board[y][x] && board[y][x] == board[y-2][x-2] && board[y-1][x-1] !=0){
       board[y-1][x-1] = board[y][x]; q.push(pair<int,int>(x-1, y-1)); draw_cursor(x-1, y-1, 1);
     }
     if (y + 2 < 8 && x + 2 < 8 && board[y+1][x+1] != board[y][x] && board[y][x] == board[y+2][x+2] && board[y+1][x+1] !=0){
       board[y+1][x+1] = board[y][x]; q.push(pair<int,int>(x+1, y+1)); draw_cursor(x+1, y+1, 1);
     }
     if (y + 2 < 8 && x - 2 >= 0 && board[y+1][x-1] != board[y][x] && board[y][x] == board[y+2][x-2] && board[y+1][x-1] !=0){
       board[y+1][x-1] = board[y][x]; q.push(pair<int,int>(x-1, y+1)); draw_cursor(x-1, y+1, 1);
     }
     if (y - 2 >= 0 && x + 2 < 8 && board[y-1][x+1] != board[y][x] && board[y][x] == board[y-2][x+2] && board[y-1][x+1] !=0){
       board[y-1][x+1] = board[y][x]; q.push(pair<int,int>(x+1, y-1)); draw_cursor(x+1, y-1, 1);
     }
  }
}

void init_game(){
  initscr();			// start curses mode 
  getmaxyx(stdscr, height, width);// get screen size

  cbreak();			// disable buffering
                                  // - use raw() to disable Ctrl-Z and Ctrl-C as well,
  halfdelay(1);			// non-blocking getch after n * 1/10 seconds
  noecho();			// disable echo
  keypad(stdscr, TRUE);		// enable function keys and arrow keys
  curs_set(0);			// hide the cursor

  init_colors();
}

void start_game(){
  clear();
  cx = cy = 3;
  init_board();
  draw_board();
  draw_cursor(cx, cy, 1);
  draw_score();
  refresh();

  attron(A_BOLD);
  move(height-1, 0);  
  printw("Arrow keys: move; Space/Enter: put; R: reset; Q: quit");
  attroff(A_BOLD);
}

int check_point(){
  int ans = 0;
  for (int i=0; i<8; i++){
    for (int j=0;j<8;j++){
      if (board[i][j] == PLAYER1) ans++;
      else if (board[i][j] == PLAYER2) ans--;
    }
  }
  return ans;
}
void determine_valid_set(){
  valid_set.clear();
  for (int i=0;i<8;i++){
    for (int j=0;j<8;j++){
      if (board[j][i] == opponent_id && i != 0 && i != 7){
        if (board[j][i-1] == self_id && board[j][i+1] == 0) valid_set.insert(pair<int,int>(i+1, j));
        else if (board[j][i-1] == 0 && board[j][i+1] == self_id) valid_set.insert(pair<int,int>(i-1, j));
      }
      if (board[j][i] == opponent_id && j != 0 && j != 7){
        if (board[j-1][i] == self_id && board[j+1][i] == 0) valid_set.insert(pair<int,int>(i, j+1));
        else if (board[j-1][i] == 0 && board[j+1][i] == self_id) valid_set.insert(pair<int,int>(i, j-1));
      }
      if (board[j][i] == opponent_id && j != 0 && j != 7 && i != 0 && i != 7){
        if (board[j-1][i-1] == self_id && board[j+1][i+1] == 0) valid_set.insert(pair<int,int>(i+1, j+1));
        else if (board[j-1][i-1] == 0 && board[j+1][i+1] == self_id) valid_set.insert(pair<int,int>(i-1, j-1));
        if (board[j-1][i+1] == self_id && board[j+1][i-1] == 0) valid_set.insert(pair<int,int>(i-1, j+1));
        else if (board[j-1][i+1] == 0 && board[j+1][i-1] == self_id) valid_set.insert(pair<int,int>(i+1, j-1));
      }
    }
  }
}

void* socket_io(void* ptr){
  int len;
  bool pass = false;
  while(1){
    pthread_mutex_lock(&thread_mutex);
    if ((len = readline(socket_fd, buffer, sizeof(buffer))) > 0 && current == opponent_id){
      int x, y;
      char* ptr = strtok(buffer, " ");
      if (strcmp(buffer, "RESET\n") == 0){
        current = RESET;
        pthread_mutex_unlock(&thread_mutex);
        continue;
      }
      else if (strcmp(ptr, "PASS\n") != 0){
        pass = false;
        x = atoi(ptr);
        y = atoi(strtok(NULL, "\n"));
        board[y][x] = opponent_id;
        draw_cursor(x, y, 1);
        check_rule(x, y);
        draw_score();
      }
      else
        pass = true;
      current = self_id;
      determine_valid_set();
      if (valid_set.size() == 0){
        writen(socket_fd, "PASS\n", 5);
        move(0, 0);
        clrtoeol();
        printw("Player #%d: Waiting for peer", self_id == PLAYER1 ? 1 : 2);
        if (pass){
          move(0, 0);
          clrtoeol();
          printw("%s", check_point() * self_id > 0 ? "You are Winner!" : "You are Loser!");
          current = 0;
        }
      }
      else{
        move(0, 0);
        clrtoeol();
        printw("Player #%d: It's my turn", self_id == PLAYER1 ? 1 : 2);
      }
    }
    else if (len > 0){
      if (strcmp(buffer, "RESET\n") == 0)
        current = RESET;
    }
    else if (len == 0){
      current = TERMINATE;
      pthread_mutex_unlock(&thread_mutex);
      pthread_exit(NULL);
    }
    pthread_mutex_unlock(&thread_mutex);
 } 
}

int main(int argc, char** argv)
{	
  parse_options(argc, argv);
  init_game();
  pthread_create(&thread, NULL, socket_io, NULL);
  pthread_detach(thread);

restart:
  start_game();
  move(0, 0);
  if (self_id == PLAYER1)
    printw("Player #1: It's my turn");
  else
    printw("Player #2: Waiting for perr");
  determine_valid_set();
  while(1){
    pthread_mutex_lock(&thread_mutex);
    int ch = getch();
    if (current == TERMINATE)
      return -1;
    else if (current == RESET){
      current = PLAYER1;
      pthread_mutex_unlock(&thread_mutex);
      goto restart;
    }
    int moved = 0;

    switch(ch){
    case ' ':
    case 0x0d:
    case 0x0a:
    case KEY_ENTER:
      if (current == self_id && valid_set.find(pair<int,int>(cx, cy)) != valid_set.end()){
        board[cy][cx] = self_id;
        draw_cursor(cx, cy, 1);
        check_rule(cx, cy);
        current = opponent_id;
        buffer[0] = (cx + '0'); buffer[1] = ' '; buffer[2] = (cy + '0'); buffer[3] = '\n';
        writen(socket_fd, buffer, 4);
        draw_score();
        move(0, 0);
        clrtoeol();
        printw("Player #%d: Waiting for peer", self_id == PLAYER1 ? 1 : 2);
        refresh();
      }
      break;
    case 'q':
    case 'Q':
      goto quit;
      break;
    case 'r':
    case 'R':
      current = RESET;
      writen(socket_fd, "RESET\n", 6);
      break;
    case 'k':
    case KEY_UP:
      draw_cursor(cx, cy, 0);
      cy = (cy-1+BOARDSZ) % BOARDSZ;
      draw_cursor(cx, cy, 1);
      moved++;
      break;
    case 'j':
    case KEY_DOWN:
      draw_cursor(cx, cy, 0);
      cy = (cy+1) % BOARDSZ;
      draw_cursor(cx, cy, 1);
      moved++;
      break;
    case 'h':
    case KEY_LEFT:
      draw_cursor(cx, cy, 0);
      cx = (cx-1+BOARDSZ) % BOARDSZ;
      draw_cursor(cx, cy, 1);
      moved++;
      break;
    case 'l':
    case KEY_RIGHT:
      draw_cursor(cx, cy, 0);
      cx = (cx+1) % BOARDSZ;
      draw_cursor(cx, cy, 1);
      moved++;
      break;
    }
    if(moved) {
      refresh();
      moved = 0;
    }
    pthread_mutex_unlock(&thread_mutex);
    napms(1);
  }

quit:
  return 0;
}
