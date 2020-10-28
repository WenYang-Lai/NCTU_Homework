#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <sstream>
#include <map>

using namespace std;

map<string, string> conditions;
string target = "";

void recursive_find(string path)
{ 
  DIR* dir = opendir(path.c_str());
  struct dirent* ptr;
  path += '/';
  while((ptr = readdir(dir)) != NULL)
  { 
    if(strcmp(ptr->d_name, ".")!=0 && strcmp(ptr->d_name, "..")!=0 && ptr->d_type == DT_DIR)
    {
      recursive_find(path+ptr->d_name);
    }
    if (strcmp(ptr->d_name, ".") !=0 && strcmp(ptr->d_name, "..") != 0)
    { 
      struct stat buf;
      stat((path+string(ptr->d_name)).c_str(), &buf);
      double file_size = (double)buf.st_size / (1024*1024);
      bool accept = true;
      for (map<string, string>::iterator it = conditions.begin(); it!=conditions.end(); it++)
      {
        if (it->first == "-name")
        {
          if (strcmp(ptr->d_name, it->second.c_str()) != 0)
          {
            accept = false;
            break;
          }
        }
        else if (it->first == "-inode")
        {
          if( (unsigned)ptr->d_ino != atoi(it->second.c_str()) )
          {
            accept = false;
            break;
          }
        }
        else if (it->first == "-size_min")
        {
          if(file_size < atoi(it->second.c_str()))
          {
            accept = false;
            break;
          }
        }
        else if (it->first == "-size_max")
        {
          if(file_size > atoi(it->second.c_str()))
          {
            accept = false;
            break;
          }
        }
        else
          fprintf(stderr, "Unknown options [%s]\n", it->first.c_str());
      }
      if (accept)
        printf("%s\t%d\t%.1f MB\n", (path+string(ptr->d_name)).c_str(), (unsigned)ptr->d_ino, file_size);
    }
  }
  closedir(dir);
}

int main(int argc, char** argv)
{
  if (argc < 4)
  {
    fprintf(stderr, "Usage: my_find [pathname] [options]\n");
    exit(1);
  }
  int i = 2;
  target = string(argv[1]);
  string key, value;
  for(;i<argc;i++)
  {
    key = string(argv[i++]);
    value = string(argv[i]);
    conditions[key] = value;
  }
  
  recursive_find(target);
  
}
