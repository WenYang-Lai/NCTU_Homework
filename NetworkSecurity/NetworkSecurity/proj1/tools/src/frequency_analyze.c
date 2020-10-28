#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <map>

using namespace std;

int main(int argc, char** argv)
{ 

  int len = atoi(argv[1]);
  printf("cipher length: %d\n", len);

  map<char, int> table[len];
  int i = 0;
  char ch;
  FILE* target_file = fopen("memo.bin", "r+b");
  int size[len] = {0};

  while ((ch = fgetc(target_file)) != EOF)
  {
    if (table[i].find(ch) != table[i].end())
      table[i][ch] ++;
    else
      table[i][ch] = 1;
    size[i] ++;
    i = (i + 1) % len ;
  }
  
  for (int i=0; i < len; i++)
  {
    printf("%dth round\n", i);
    for (map<char, int>::iterator it = table[i].begin(); it != table[i].end(); it++)
    {
      printf("\tchar: %02x, frequency: %f\n", (unsigned char)it->first, (((float)it->second)/size[i])*100);
    }
  }
  fclose(target_file);

}
