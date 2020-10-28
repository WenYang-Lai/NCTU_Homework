#include <map>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <algorithm>

using namespace std;

#define IS_FUNCTION 0x1
#define IS_ARRAY    0x2
#define IS_DECLARE  0x4

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

extern vector< vector<struct Symbol*> > semanticMap;
extern vector<struct Symbol*> semanticBuffer;
extern vector<struct Symbol*> arrayInitBuffer;
extern vector<int> exprCountVector;

extern int eq_cmp;
extern int array_sz;
extern int actual_sz;
extern int loop_count;
extern int expr_list;
extern int error_count;

extern string ret_type_str;
extern string scalar_type_str;
#endif

int printSymTable();
void createNewSym();
void insertSymMap();
void incLevel();
void decLevel();

// project 4
struct Symbol* getSymbol(char* symbol_name, int flag, int level);
void undefinedError(char*);
void typeMissError();
void conditionTypeError();
void returnTypeError();
void arrayInitError();
void constAssignError();
void jumpError();
void arrayDeclError();
void functionInvokeMissError();

void isArrayCheck();
int conditionCheck();
int initArrayCheck();
struct Symbol* arithCheck(int is_binary, int is_relation );
struct Symbol* logicalCheck(int is_binary);
struct Symbol* assignCheck(int flag=0);
