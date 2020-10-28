#include <dlfcn.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <dirent.h>
#include <stdarg.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <limits.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

#define EMPTY_STRING  ""
#define NULL_STRING "(null)"

#define SET_FP() \
  if ( out == NULL ) set_output_file();

#define INJECT_SYMBOL(lib, target, symbol, type) \
  if ( target == NULL ){ \
    void* handle; \
    handle = dlopen(lib, RTLD_LAZY); \
    if(handle == NULL) \
      fprintf(stderr, "Can't open %s, error: %s\n", lib, dlerror()); \
    target = (type)dlsym( handle, symbol ); \
    if ( target == NULL) \
      fprintf(stderr, "link lib %s failed\n", symbol ); \
    SET_FP(); \
  }  

#define PARSE_ARGV \
  va_list ap, ap_count; \
  va_start(ap, argv); \
  va_copy(ap_count, ap); \
  int count = 0; \
  const char* tmp = argv; \
  while(tmp != NULL){ \
    count++; \
    tmp = va_arg(ap_count, const char*); \
  } \
  char* new_argv[count+1]; \
  int count_2 = 0; tmp = argv; \
  while (count_2 <= count){ \
    new_argv[count++] = (char*)tmp; \
    tmp = va_arg(ap_count, char*); \
  }

/* general proto */
FILE* out = NULL;
void open_dl(void*,const char*);


/* dirent.h proto */
static int (*old_closedir)(DIR*) = NULL;
static DIR* (*old_fdopendir)(int) = NULL;
static DIR* (*old_opendir)(const char*) = NULL;
static struct dirent* (*old_readdir)(DIR*) = NULL;
static int (*old_readdir_r)(DIR*, struct dirent*, struct dirent**) = NULL;
static void (*old_rewinddir)(DIR*) = NULL;
static void (*old_seekdir)(DIR*, long) = NULL;
static long (*old_telldir)(DIR*) = NULL;

/* fcntl.h proto */
static int (*old_creat)(const char*, mode_t) = NULL;
static int (*old_open)(const char*, int, ...) = NULL;

/* stdio.h proto */
static int (*old_remove)(const char*) = NULL;
static int (*old_rename)(const char*, const char*) = NULL;
static void (*old_setbuf)(FILE*, char*) = NULL;
static int (*old_setvbuf)(FILE*, char*, int, size_t) = NULL;
static char* (*old_tempnam)(const char*, const char*) = NULL;
static FILE* (*old_tmpfile)(void) = NULL;
static char* (*old_tmpnam)(char*) = NULL;

/* stdlib.h proto */
static void (*old_exit)(int) = NULL;
static char* (*old_getenv)(const char*) = NULL;
static char* (*old_mkdtemp)(char*) = NULL;
static int (*old_mkstemp)(char*) = NULL;
static int (*old_putenv)(char*) = NULL;
static int (*old_rand)(void) = NULL;
static int (*old_rand_r)(unsigned*) = NULL;
static int (*old_setenv)(const char*, const char*, int) = NULL;
static void (*old_srand)(unsigned) = NULL;
static int (*old_system)(const char*) = NULL;

/* unistd.h proto */ 
static int (*old_chdir)(const char*) = NULL;
static int (*old_chown)(const char*, uid_t, gid_t) = NULL;
static int (*old_close)(int) = NULL;
static int (*old_dup)(int) = NULL;
static int (*old_dup2)(int,int) = NULL;
static void (*old__exit)(int) = NULL;
static int (*old_execl)(const char*, const char*, ...) = NULL;
static int (*old_execle)(const char*, const char*, ...) = NULL;
static int (*old_execlp)(const char*, const char*, ...) = NULL;
static int (*old_execv)(const char*, char *const []) = NULL;
static int (*old_execve)(const char*, char *const [], char *const []) = NULL;
static int (*old_execvp)(const char*, char *const []) = NULL;
static int (*old_fchdir)(int) = NULL;
static int (*old_fchown)(int, uid_t, gid_t) = NULL;
static pid_t (*old_fork)(void) = NULL;
static int (*old_fsync)(int) = NULL;
static int (*old_ftruncate)(int, off_t) = NULL;
static char* (*old_getcwd)(char*, size_t) = NULL;
static gid_t (*old_getegid)(void) = NULL;
static uid_t (*old_geteuid)(void) = NULL;
static gid_t (*old_getgid)(void) = NULL;
static uid_t (*old_getuid)(void) = NULL;
static int (*old_link)(const char*, const char*) = NULL;
static int (*old_pipe)(int [2]) = NULL;
static ssize_t (*old_pread)(int, void*, size_t, off_t) = NULL;
static ssize_t (*old_pwrite)(int, const void*, size_t, off_t) = NULL;
static ssize_t (*old_read)(int, void*, size_t) = NULL;
static ssize_t (*old_readlink)(const char*, char*, size_t) = NULL;
static int (*old_rmdir)(const char*) = NULL;
static int (*old_setegid)(gid_t) = NULL;
static int (*old_seteuid)(uid_t) = NULL;
static int (*old_setgid)(gid_t) = NULL;
static int (*old_setuid)(uid_t) = NULL;
static unsigned (*old_sleep)(unsigned) = NULL;
static int (*old_symlink)(const char*, const char*) = NULL;
static int (*old_unlink)(const char*) = NULL;
static ssize_t (*old_write)(int, const void*, size_t) = NULL; 

/* sys_stat.h */
static int (*old_chmod)(const char*, mode_t) = NULL;
static int (*old_fchmod)(int, mode_t) = NULL;
static int (*old_fstat)(int, int, struct stat*) = NULL;
static int (*old_lstat)(int, const char*, struct stat*) = NULL;
static int (*old_mkdir)(const char*, mode_t) = NULL;
static int (*old_mkfifo)(const char*, mode_t) = NULL;
static int (*old_stat)(int, const char*, struct stat*) = NULL;
static mode_t (*old_umask)(mode_t) = NULL;

/* addition part */
static int (*old_accept)(int, struct sockaddr*, socklen_t*) = NULL;
static int (*old_bind)(int, const struct sockaddr*, socklen_t) = NULL;
static int (*old_connect)(int, const struct sockaddr*, socklen_t) = NULL;
static int (*old_socket)(int, int, int) = NULL;
static int (*old_setsockopt)(int, int, int, const void*, socklen_t) = NULL;


/* general function */ 
void set_output_file(){
  char* file = old_getenv == NULL ? getenv("MONITOR_OUTPUT") : old_getenv("MONITOR_OUTPUT");
  if (out != NULL)
    return;
  if (strcmp(file, "stderr") == 0)
    out = stderr;
  else if (strcmp(file, "stdout") == 0)
    out = stdout;
  else
    out = fopen(file ,"w");
}
char* get_sock_info(const struct sockaddr* addr){
  char *ip = (char*)malloc(sizeof(char)*256);
  if ( addr->sa_family == AF_INET6 ){
    struct in6_addr addr_6 = ((struct sockaddr_in6*)addr)->sin6_addr;
    inet_ntop(AF_INET6, &addr_6, ip, INET6_ADDRSTRLEN);
  }
  else if( addr->sa_family == AF_INET ){
    struct in_addr addr_4 = ((struct sockaddr_in*)addr)->sin_addr;
    inet_ntop(AF_INET, &addr_4, ip, INET_ADDRSTRLEN);
  }
  else if (addr->sa_family == AF_UNIX)
    strcpy(ip, "DOMAIN SOCKET");
  else
    strcpy(ip, "Unknown type!");
  return ip;
}

/* dirent.h injection implementation */
int closedir(DIR* dir){ 
  INJECT_SYMBOL("libc.so.6", old_closedir, "closedir", int(*)(DIR*));
  int ret = (*old_closedir)(dir);
  fprintf(out,"[monitor] closedir(%p) = %d\n", dir, ret);
  return ret;
}


DIR* fdopendir(int fd){
  INJECT_SYMBOL("libc.so.6", old_fdopendir, "fdopendir", DIR*(*)(int));
  DIR* ret = old_fdopendir(fd);
  fprintf(out,"[monitor] fdopendir(%d) = %p\n", fd, ret);
  return ret;
}

DIR* opendir(const char* path){
  INJECT_SYMBOL("libc.so.6", old_opendir, "opendir", DIR*(*)(const char*));
  DIR* ret = old_opendir(path);
  fprintf(out,"[monitor] opendir('%s') = %p\n", path, ret);
  return ret;
}

struct dirent* readdir(DIR* dir){
  INJECT_SYMBOL("libc.so.6", old_readdir, "readdir", struct dirent*(*)(DIR*));
  struct dirent* ret = old_readdir(dir);
  fprintf(out,"[monitor] readdir(%p) = %p\n", dir, ret);
  return ret;
}

int readdir_r(DIR* dir, struct dirent* ptr, struct dirent** d_ptr){
  INJECT_SYMBOL("libc.so.6", old_readdir_r, "readdir_r", int(*)(DIR*, struct dirent*, struct dirent**));
  int ret = old_readdir_r(dir, ptr, d_ptr);
  fprintf(out,"[monitor] readdir_r(%p, %p, %p) = %d\n", dir, ptr, d_ptr, ret);
  return ret;
}

void rewinddir(DIR* dir){
  INJECT_SYMBOL("libc.so.6", old_rewinddir, "rewinddir", void(*)(DIR*));
  old_rewinddir(dir);
  fprintf(out, "[monitor] rewinddir(%p)\n", dir);
  return ;
}

void seekdir(DIR* dir, long num){
  INJECT_SYMBOL("libc.so.6", old_seekdir, "seekdir", void(*)(DIR*,long));
  old_seekdir(dir, num);
  fprintf(out, "[monitor] seekdir(%p, %ld)\n", dir, num);
  return ;
}

long telldir(DIR* dir){
  INJECT_SYMBOL("libc.so.6", old_telldir, "telldir", long(*)(DIR*));
  long ret = old_telldir(dir);
  fprintf(out, "[monitor] telldir(%p) = %ld\n", dir, ret);
  return ret;
}

/* fcntl.h injection implementation*/
int creat(const char* name, mode_t mode){
  INJECT_SYMBOL("libc.so.6", old_creat, "creat", int(*)(const char*, mode_t));
  int ret = old_creat(name, mode);
  fprintf(out, "[monitor] creat('%s', %d) = %d\n", name, mode, ret);
  return ret;
}

int open(const char* path, int oflag, ...){
  INJECT_SYMBOL("libc.so.6", old_open, "open", int(*)(const char*, int, ...));
  int ret;
  if (oflag & O_CREAT){
    va_list ap;
    va_start(ap, oflag);
    mode_t mode = va_arg(ap, mode_t);
    ret = old_open(path, oflag, mode);
    fprintf(out, "[monitor] open('%s', %d, %d) = %d\n", path, oflag, mode, ret);
  }
  else{
    ret = old_open(path, oflag);
    fprintf(out, "[monitor] open('%s', %d) = %d\n", path, oflag, ret);
  }
  return ret;
}


/* stdio.h injection implementation */
int remove(const char* path){
  INJECT_SYMBOL("libc.so.6", old_remove, "remove", int(*)(const char*));
  int ret = old_remove(path);
  fprintf(out, "[monitor] remove('%s') = %d\n", path, ret);
  return ret;
}

int rename(const char* old, const char* new_name){
  INJECT_SYMBOL("libc.so.6", old_rename, "rename", int(*)(const char*, const char*));
  int ret = old_rename(old, new_name);
  fprintf(out, "[monitor] rename('%s', '%s') = %d\n", old, new_name, ret);
  return ret;
}

void setbuf(FILE* file, char* buf){
  INJECT_SYMBOL("libc.so.6", old_setbuf, "setbuf", void(*)(FILE*, char*));
  old_setbuf(file, buf);
  fprintf(out, "[monitor] setbuf(%p, '%s')\n", file, buf);
  return ;
}
int setvbuf(FILE* file, char* buf, int flag, size_t size){
  INJECT_SYMBOL("libc.so.6", old_setvbuf, "setvbuf", int(*)(FILE*, char*, int, size_t));
  int ret = old_setvbuf(file, buf, flag, size);
  fprintf(out, "[monitor] setvbuf(%p, '%s', %d, %ld) = %d\n", file, buf, flag, size, ret);
  return ret;
}

char* tempnam(const char* name1, const char* name2){
  INJECT_SYMBOL("libc.so.6", old_tempnam, "tempnam", char*(*)(const char*, const char*));
  char* ret = old_tempnam(name1, name2);
  fprintf(out, "[monitor] tempnam('%s', '%s') = '%s'\n", name1, name2, ret == NULL ? NULL_STRING : ret);
  return ret;
}
FILE* tmpfile(void){
  INJECT_SYMBOL("libc.so.6", old_tmpfile, "tmpfile", FILE*(*)(void));
  FILE* ret = old_tmpfile();
  fprintf(out, "[monitor] tmpfile() = %p\n", ret);
  return ret;
}

char* tmpnam(char* name){
  INJECT_SYMBOL("libc.so.6", old_tmpnam, "tmpnam", char*(*)(char*));
  char* ret = old_tmpnam(name);
  fprintf(out, "[monitor] tmpnam('%s') = '%s'\n", name, ret == NULL ? NULL_STRING : ret);
  return ret;
}


/* stdlib.h injection implementaton */
void exit(int status){
  INJECT_SYMBOL("libc.so.6", old_exit, "exit", void(*)(int));
  fprintf(out, "[monitor] exit(%d)\n", status);
  old_exit(status);
}

char* getenv(const char* var){
  INJECT_SYMBOL("libc.so.6", old_getenv, "getenv", char*(*)(const char*));
  char* ret = old_getenv(var);
  fprintf(out, "[monitor] getenv('%s') = '%s'\n", var == NULL ? NULL_STRING : var, ret == NULL ? NULL_STRING : ret);
  return ret;
}

char* mkdtemp(char* path){
  INJECT_SYMBOL("libc.so.6", old_mkdtemp, "mkdtemp", char*(*)(char*));
  char* ret = old_mkdtemp(path);
  fprintf(out, "[monitor] mkdtemp('%s') = '%s'\n", path, ret == NULL ? NULL_STRING : ret);
  return ret;
}

int mkstemp(char* path){
  INJECT_SYMBOL("libc.so.6", old_mkstemp, "mkstemp", int(*)(char*));
  int ret = old_mkstemp(path);
  fprintf(out, "[monitor] mkstemp('%s') = %d\n", path, ret);
  return ret;
}

int putenv(char* str){
  INJECT_SYMBOL("libc.so.6", old_putenv, "putenv", int(*)(char*));
  int ret = old_putenv(str);
  fprintf(out, "[monitor] putenv('%s') = %d\n", str, ret);
  return ret;
}

int rand(void){
  INJECT_SYMBOL("libc.so.6", old_rand, "rand", int(*)(void));
  int ret = old_rand();
  fprintf(out, "[monitor] rand() = %d\n", ret);
  return ret;
}

int rand_r(unsigned* var){
  INJECT_SYMBOL("libc.so.6", old_rand_r, "rand_r", int(*)(unsigned*));
  int ret = old_rand_r(var);
  fprintf(out, "[monitor] rand_r('%p') = %d\n", var, ret);
  return ret;
}

int setenv(const char* env, const char* val, int flag){
  INJECT_SYMBOL("libc.so.6", old_setenv, "setenv", int(*)(const char*, const char*, int));
  int ret = old_setenv(env, val, flag);
  fprintf(out, "[monitor] setenv('%s', '%s', %d) = %d\n", env, val, flag, ret);
  return ret;
}

void srand(unsigned seed){
  INJECT_SYMBOL("libc.so.6", old_srand, "srand", void(*)(unsigned));
  old_srand(seed);
  fprintf(out, "[monitor] srand(%u)\n", seed);
  return ;

}
int system(const char* cmd){
  INJECT_SYMBOL("libc.so.6", old_system, "system", int(*)(const char*));
  int ret = old_system(cmd);
  fprintf(out, "[monitor] system('%s') = %d\n", cmd, ret);
  return ret;

}


/* unistd.h injection implementation */
int chdir(const char* path){
  INJECT_SYMBOL("libc.so.6", old_chdir, "chdir", int(*)(const char*));
  int ret = old_chdir(path);
  fprintf(out, "[monitor] chdir('%s') = %d\n", path, ret);
  return ret;
}

int chown(const char* path, uid_t uid, gid_t gid){
  INJECT_SYMBOL("libc.so.6", old_chown, "chown", int(*)(const char*, uid_t, gid_t));
  int ret = old_chown(path, uid, gid);
  fprintf(out, "[monitor] chown('%s', %u, %u) = %d\n", path, uid, gid, ret);
  return ret;
}

int close(int fd){
  INJECT_SYMBOL("libc.so.6", old_close, "close", int(*)(int));
  int ret = old_close(fd);
  fprintf(out, "[monitor] close(%d) = %d\n", fd, ret);
  return ret;
}

int dup(int fd){
  INJECT_SYMBOL("libc.so.6", old_dup, "dup", int(*)(int));
  int ret = old_dup(fd);
  fprintf(out, "[monitor] dup(%d) = %d\n", fd, ret);
  return ret;
}

int dup2(int fd1,int fd2){
  INJECT_SYMBOL("libc.so.6", old_dup2, "dup2", int(*)(int,int));
  int ret = old_dup2(fd1, fd2);
  fprintf(out, "[monitor] dup2(%d, %d) = %d\n", fd1, fd2, ret);
  return ret;
}

void _exit(int status){
  INJECT_SYMBOL("libc.so.6", old__exit, "_exit", void(*)(int));
  fprintf(out, "[monitor] _exit(%d)\n", status);
  old__exit(status);
}

int execv(const char* path, char *const cmd[]){
  INJECT_SYMBOL("libc.so.6", old_execv, "execn", int(*)(const char*, char *const []));
  int ret = old_execv(path, cmd);
  fprintf(out, "[monitor] execv('%s', ('%s', ... )) = %d\n", path, cmd[0] == NULL ? NULL_STRING : cmd[0], ret);
  return ret;
}
int execve(const char* path, char *const cmd1[], char *const cmd2[]){
  INJECT_SYMBOL("libc.so.6", old_execve, "execve", int(*)(const char* ,char *const [], char *const []));
  int ret = old_execve(path, cmd1, cmd2);
  fprintf(out, "[monitor] execve('%s', ('%s', ...), ('%s', ...)) = %d\n", path, cmd1[0] == NULL ? NULL_STRING : cmd1[0], cmd2[0] == NULL ? NULL_STRING : cmd2[0], ret);
  return ret;
}

int execvp(const char* path, char *const argv[]){
  INJECT_SYMBOL("libc.so.6", old_execvp, "execvp", int(*)(const char*, char *const []));
  int ret = old_execvp(path, argv);
  fprintf(out, "[monitor] execvp('%s', '%s', ...) = %d\n", path, argv[0] == NULL ? NULL_STRING : argv[0], ret);
  return ret;
}

int execl(const char* path, const char* argv, ...){
  PARSE_ARGV
  int ret = execv(path, new_argv);
  fprintf(out, "[monitor] execl('%s', '%s', ...) = %d\n", path, argv == NULL ? NULL_STRING : argv, ret);
  return ret;
}

int execle(const char* path, const char* argv, ...){
  int argc;
  va_list ap;
  va_start (ap, argv);
  for (argc = 1; va_arg (ap, const char *); argc++)
    {
      if (argc == INT_MAX)
	{
	  va_end (ap);
	  errno = E2BIG;
	  return -1;
	}
    }
  va_end (ap);
  int i;
  char *new_argv[argc + 1];
  char **envp;
  va_start (ap, argv);
  new_argv[0] = (char *) argv;
  for (i = 1; i <= argc; i++)
    new_argv[i] = va_arg (ap, char *);
  envp = va_arg (ap, char **);
  va_end (ap);

  int ret = execve(path, new_argv, envp);
  fprintf(out, "[monitor] execle('%s', ('%s', ...), ('%s', ...)) = %d\n", path, argv == NULL ? NULL_STRING : argv, envp[0] == NULL ? NULL_STRING : envp[0], ret);
  return ret;
}

int execlp(const char* path, const char* argv, ...){
  PARSE_ARGV 
  int ret = execvp(path, new_argv);
  fprintf(out, "[monitor] execlp('%s', '%s', ...) = %d\n", path, argv == NULL ? NULL_STRING : argv, ret);
  return ret;
}

int fchdir(int fd){
  INJECT_SYMBOL("libc.so.6", old_fchdir, "fchdir", int(*)(int));
  int ret = old_fchdir(fd);
  fprintf(out, "[monitor] fchdir(%d) = %d\n", fd, ret);
  return ret;
}

int fchown(int fd, uid_t uid, gid_t gid){
  INJECT_SYMBOL("libc.so.6", old_fchown, "fchown", int(*)(int, uid_t, gid_t));
  int ret = old_fchown(fd, uid, gid);
  fprintf(out, "[monitor] fchown(%d, %d, %d) = %d\n", fd, uid, gid, ret);
  return ret;
}

pid_t fork(void){
  INJECT_SYMBOL("libc.so.6", old_fork, "fork", int(*)(void));
  int ret = old_fork();
  fprintf(out, "[monitor] fork() = %d\n",  ret);
  return ret;
}

int fsync(int fd){
  INJECT_SYMBOL("libc.so.6", old_fsync, "fsync", int(*)(int));
  int ret = old_fsync(fd);
  fprintf(out, "[monitor] fsync(%d) = %d\n", fd, ret);
  return ret;
}

int ftruncate(int fd, off_t offset){
  INJECT_SYMBOL("libc.so.6", old_ftruncate, "ftruncate", int(*)(int, off_t));
  int ret = old_ftruncate(fd, offset);
  fprintf(out, "[monitor] ftruncate(%d, %lu) = %d\n", fd, offset, ret);
  return ret;
}

char* getcwd(char* path, size_t size){
  INJECT_SYMBOL("libc.so.6", old_getcwd, "getcwd", char*(*)(char*, size_t));
  char* ret = old_getcwd(path, size);
  fprintf(out, "[monitor] getcwd('%s', %lu) = %s\n", path, size, ret == NULL ? NULL_STRING : ret);
  return ret;
}

gid_t getegid(void){
  INJECT_SYMBOL("libc.so.6", old_getegid, "getegid", gid_t(*)(void));
  int ret = old_getegid();
  fprintf(out, "[monitor] getegid() = %u\n", ret);
  return ret;
}

uid_t geteuid(void){
  INJECT_SYMBOL("libc.so.6", old_geteuid, "geteuid", uid_t(*)(void));
  int ret = old_geteuid();
  fprintf(out, "[monitor] geteuid() = %d\n", ret);
  return ret;
}

gid_t getgid(void){
  INJECT_SYMBOL("libc.so.6", old_getgid, "getgid", gid_t(*)(void));
  int ret = old_getgid();
  fprintf(out, "[monitor] getgid() = %d\n", ret);
  return ret;
}

uid_t getuid(void){
  INJECT_SYMBOL("libc.so.6", old_getuid, "getuid", uid_t(*)(void));
  uid_t ret = old_getuid();
  fprintf(out, "[monitor] getuid() = %d\n", ret);
  return ret;
}

int link(const char* target, const char* src){
  INJECT_SYMBOL("libc.so.6", old_link, "link", int(*)(const char*, const char*));
  int ret = old_link(target, src);
  fprintf(out, "[monitor] link('%s', '%s') = %d\n", target, src, ret);
  return ret;
}

int pipe(int fd[2]){
  INJECT_SYMBOL("libc.so.6", old_pipe, "pipe", int(*)(int [2]));
  int ret = old_pipe(fd);
  fprintf(out, "[monitor] pipe(%p) = %d\n", fd, ret);
  return ret;
}

ssize_t pread(int n, void* buf, size_t size, off_t offset){
  INJECT_SYMBOL("libc.so.6", old_pread, "pread", ssize_t(*)(int, void*, size_t, off_t));
  ssize_t ret = old_pread(n, buf, size, offset);
  fprintf(out, "[monitor] getuid(%d, %p, %lu, %lu) = %lu\n", n, buf, size, offset, ret);
  return ret;
}

ssize_t pwrite(int n, const void* buf, size_t size, off_t offset){
  INJECT_SYMBOL("libc.so.6", old_pwrite, "pwrite", ssize_t(*)(int, const void*, size_t, off_t));
  ssize_t ret = old_pwrite(n, buf, size, offset);
  fprintf(out, "[monitor] pwrite(%d, %p, %lu, %lu) = %lu\n", n, buf, size, offset, ret);
  return ret;
}

ssize_t read(int fd, void* buf, size_t len){
  INJECT_SYMBOL("libc.so.6", old_read, "read", ssize_t(*)(int, void*, size_t));
  ssize_t ret = old_read(fd, buf, len);
  fprintf(out, "[monitor] read(%d, %p, %lu) = %ld\n", fd, buf, len, ret);
  return ret;
}

ssize_t readlink(const char* path, char* n, size_t len){
  INJECT_SYMBOL("libc.so.6", old_readlink, "readlink", ssize_t(*)(const char*, char*, size_t));
  ssize_t ret = old_readlink(path, n, len);
  fprintf(out, "[monitor] readlink('%s', '%s', %lu) = %lu\n", path, n, len, ret);
  return ret;
}

int rmdir(const char* path){
  INJECT_SYMBOL("libc.so.6", old_rmdir, "rmdir", int(*)(const char*));
  int ret = old_rmdir(path);
  fprintf(out, "[monitor] rmdir('%s') = %d\n", path, ret);
  return ret;
}

int setegid(gid_t gid){
  INJECT_SYMBOL("libc.so.6", old_setegid, "setegid", int(*)(gid_t));
  int ret = old_setegid(gid);
  fprintf(out, "[monitor] setegid(%u) = %d\n", gid, ret);
  return ret;
}

int seteuid(uid_t uid){
  INJECT_SYMBOL("libc.so.6", old_seteuid, "seteuid", int(*)(uid_t));
  int ret = old_seteuid(uid);
  fprintf(out, "[monitor] seteuid(%u) = %d\n", uid, ret);
  return ret;
}

int setgid(gid_t gid){
  INJECT_SYMBOL("libc.so.6", old_setgid, "setgid", int(*)(gid_t));
  int ret = old_setgid(gid);
  fprintf(out, "[monitor] setgid(%u) = %d\n", gid, ret);
  return ret;
}

int setuid(uid_t uid){
  INJECT_SYMBOL("libc.so.6", old_setuid, "setuid", int(*)(uid_t));
  int ret = old_setuid(uid);
  fprintf(out, "[monitor] setuid(%d) = %d\n", uid, ret);
  return ret;
}
unsigned sleep(unsigned sec){
  INJECT_SYMBOL("libc.so.6", old_sleep, "sleep", unsigned(*)(unsigned));
  unsigned ret = old_sleep(sec);
  fprintf(out, "[monitor] sleep(%u) = %d\n", sec, ret);
  return ret;
}

int symlink(const char* dst, const char* src){
  INJECT_SYMBOL("libc.so.6", old_symlink, "symlink", int(*)(const char*, const char*));
  int ret = old_symlink(dst, src);
  fprintf(out, "[monitor] symlink('%s', '%s') = %d\n", dst, src, ret);
  return ret;
}

int unlink(const char* path){
  INJECT_SYMBOL("libc.so.6", old_unlink, "unlink", int(*)(const char*));
  int ret = old_unlink(path);
  fprintf(out, "[monitor] unlink('%s') = %d\n", path, ret);
  return ret;
}

ssize_t write(int fd, const void* buf, size_t len){
  INJECT_SYMBOL("libc.so.6", old_write, "write", ssize_t(*)(int, const void*, size_t));
  int ret = old_write(fd, buf, len);
  fprintf(out, "[monitor] write(%d, %p, %lu) = %d\n", fd, buf, len, ret);
  return ret;
}


/* sys_stat.h injection implementation */
int chmod(const char* path, mode_t mode){
  INJECT_SYMBOL("libc.so.6", old_chmod, "chmod", int(*)(const char*, mode_t));
  int ret = old_chmod(path, mode);
  fprintf(out, "[monitor] chmod('%s', %d) = %d\n", path, mode, ret);
  return ret;
}

int fchmod(int n, mode_t mode){
  INJECT_SYMBOL("libc.so.6", old_fchmod, "fchmod", int(*)(int, mode_t));
  int ret = old_fchmod(n, mode);
  fprintf(out, "[monitor] fchmod(%d, %u) = %d\n", n, mode, ret);
  return ret;
}

int __fxstat(int ver, int fd, struct stat* st){
  INJECT_SYMBOL("libc.so.6", old_fstat, "__fxstat", int(*)(int, int, struct stat*));
  int ret = old_fstat(ver, fd, st);
  fprintf(out, "[monitor] __fxstat(%d, %d, %p) = %d\n", ver, fd, st, ret);
  return ret;
}

int __lxstat(int version, const char* path, struct stat* st){
  INJECT_SYMBOL("libc.so.6", old_lstat, "__lxstat", int(*)(int, const char*, struct stat*));
  int ret = old_lstat(version, path, st);
  fprintf(out, "[monitor] __lxstat(%d, '%s', %p) = %d\n", version, path, st, ret);
  return ret;
}

int mkdir(const char* name, mode_t mode){
  INJECT_SYMBOL("libc.so.6", old_mkdir, "mkdir", int(*)(const char*, mode_t));
  int ret = old_mkdir(name, mode);
  fprintf(out, "[monitor] mkdir('%s', %d) = %d\n", name, mode, ret);
  return ret;
}

int mkfifo(const char* name, mode_t mode){
  INJECT_SYMBOL("libc.so.6", old_mkfifo, "mkfifo", int(*)(const char*, mode_t));
  int ret = old_mkfifo(name, mode);
  fprintf(out, "[monitor] mkfifo('%s', %d) = %d\n", name, mode, ret);
  return ret;
}

int __xstat(int ver, const char* path, struct stat* st){
  INJECT_SYMBOL("libc.so.6", old_stat, "__xstat", int(*)(int,const char*, struct stat*));
  int ret = old_stat(ver, path, st);
  fprintf(out, "[monitor] __xstat(%d, '%s', %p) = %d\n", ver, path, st, ret);
  return ret;
}

mode_t umask(mode_t mode){
  INJECT_SYMBOL("libc.so.6", old_umask, "umask", mode_t(*)(mode_t));
  int ret = old_umask(mode);
  fprintf(out, "[monitor] umask(%d) = %d\n", mode, ret);
  return ret;
}

/* addition part injection implementation */
int accept(int fd, struct sockaddr* addr, socklen_t* len){
  INJECT_SYMBOL("libc.so.6", old_accept, "accept", int(*)(int, struct sockaddr*, socklen_t*));
  int ret = old_accept(fd, addr, len);
  char *ip = get_sock_info(addr);  
  fprintf(out, "[monitor] accept(%d, '%s', %u) = %d\n", fd, ip, *len, ret);
  free(ip);
  return ret;
}

int bind(int fd, const struct sockaddr* addr , socklen_t len){
  INJECT_SYMBOL("libc.so.6", old_bind, "bind", int(*)(int, const struct sockaddr*, socklen_t));
  int ret = old_bind(fd, addr, len);
  char *ip = get_sock_info(addr);  
  fprintf(out, "[monitor] bind(%d, '%s', %u) = %d\n", fd, ip, len, ret);
  free(ip);
  return ret;
}

int connect(int fd, const struct sockaddr* addr, socklen_t len){
  INJECT_SYMBOL("libc.so.6", old_connect, "connect", int(*)(int, const struct sockaddr*, socklen_t));
  int ret = old_connect(fd, addr, len);
  char *ip = get_sock_info(addr);  
  fprintf(out, "[monitor] connect(%d, '%s', %u) = %d\n", fd, ip, len, ret);
  free(ip);
  return ret;
}

int socket(int domain, int socket_flag, int protocol){
  INJECT_SYMBOL("libc.so.6", old_socket, "socket", int(*)(int, int, int));
  int ret = old_socket(domain, socket_flag, protocol);
  fprintf(out, "[monitor] socket(%d, %d, %d) = %d\n", domain, socket_flag, protocol, ret);
  return ret;
}

int setsockopt(int fd, int level, int optname, const void* optval, socklen_t optlen){
  INJECT_SYMBOL("libc.so.6", old_setsockopt, "setsockopt", int(*)(int, int, int, const void*, socklen_t));
  int ret = old_setsockopt(fd ,level, optname, optval, optlen);
  fprintf(out, "[monitor] setsockopt(%d, %d, %d, %p, %u) = %d\n", fd, level, optname, optval, optlen, ret);
  return ret;
}

