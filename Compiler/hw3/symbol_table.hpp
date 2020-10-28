#include <map>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector>
using namespace std;

extern "C"{
    extern int linenum;
    extern int Opt_Symbol;
}

struct Symbol{
    char* name;
    char* kind;
    int level;
    char* type;
    char* attribute;
    int is_declare;
};

#ifndef _DECLARE
extern struct Symbol* cur_sym;
extern int cur_level;

extern vector< vector<struct Symbol*> > symbolMap;
extern vector<struct Symbol*> symbolBuffer;
#endif

void createNewSym();
void insertSymMap();
void incLevel();
void decLevel();
int printSymTable();

