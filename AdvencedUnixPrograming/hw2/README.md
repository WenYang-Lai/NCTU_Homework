# library hijacking

## Include functions.

### basic assignment requirement

```
  closedir fdopendir opendir readdir readdir_r rewinddir seekdir telldir creat open remove rename setbuf setvbuf tempnam tmpfile tmpnam exit getenv mkdtemp mkstemp putenv rand rand_r setenv srand system chdir chown close dup dup2 _exit execl execle execlp execv execve execvp fchdir fchown fork fsync ftruncate getcwd getegid geteuid getgid getuid link pipe pread pwrite read readlink rmdir setegid seteuid setgid setuid sleep symlink unlink write chmod fchmod fstat lstat mkdir mkfifo stat umask
```
### addition functions

Using for introspect the network api to retreive socket information.

```
  accept(), bind(), socket(), connect(), setsockopt()
```

