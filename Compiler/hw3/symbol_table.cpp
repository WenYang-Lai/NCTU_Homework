#define _DECLARE
#include "symbol_table.hpp"

struct Symbol* cur_sym = NULL;
int cur_level = 0;

vector< vector<struct Symbol*> > symbolMap;
vector<struct Symbol*> symbolBuffer;

void incLevel(){
    cur_level++;
    symbolMap.push_back(vector<struct Symbol*>());
}

void decLevel(){
    cur_level--;
    if(Opt_Symbol)
        printSymTable();
    symbolMap.pop_back();
}

void createNewSym(){
    cur_sym = (struct Symbol*)calloc(sizeof(struct Symbol), 1);
}

void insertSymMap(){
    cur_sym->level = cur_level;
    vector<struct Symbol*> top = symbolMap.back();
    int sz = top.size();
    for (int i = 0; i < sz ; i++){
        bool sym_same = (strcmp(top[i]->name, cur_sym->name) == 0);
        if(sym_same && top[i]->is_declare == 1 && cur_sym->is_declare == 0 ){
            top[i]->is_declare == 0;
            return;
        }
        else if(sym_same){
            printf("########## Error at Line#%d: symbol %s is redeclared ##########\n", linenum, cur_sym->name);
            return;
        }
    }

    top.push_back(cur_sym);

    symbolMap.pop_back();
    symbolMap.push_back(top);
}

//copy this output format to your program
int printSymTable(){

    char* level;
    const char* attribute;
    vector<struct Symbol*> top = symbolMap.back();
    size_t sz = top.size();

    if(sz == 0){
        return 0;
    } 
    
    printf("=======================================================================================\n");
    printf("Name                             Kind       Level       Type               Attribute   \n");
    printf("---------------------------------------------------------------------------------------\n");

    for (int i = 0; i < sz; i++){
        asprintf(&level, "%d(%s)", top[i]->level, top[i]->level ==0 ? "global" : "local");
        attribute = top[i]->attribute ? top[i]->attribute : "";
        printf("%-32s %-11s%-12s%-19s%s\n",top[i]->name,top[i]->kind,level,top[i]->type, attribute);
        free(level);
    }

    printf("=======================================================================================\n");
}
