#include <unistd.h>
#include <stdio.h>

#define cipher_length   7

char array[7] = {0x62, 0x75, 0x79, 0x6f, 0x75, 0x74, 0x73};

int main()
{
  FILE* ifile = fopen("memo.bin", "r+b");
  FILE* ofile = fopen("result.txt", "w");
  char ch;
    
  int i=0;
  while ((ch = fgetc(ifile)) != EOF)
  { 
    fprintf(ofile, "%c", ch ^ array[i]);
    i = (i + 1) % cipher_length;
  }
  fclose(ifile);
  fclose(ofile);
}
