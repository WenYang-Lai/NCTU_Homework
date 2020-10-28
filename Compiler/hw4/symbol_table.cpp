#define _DECLARE
#include "symbol_table.hpp"

struct Symbol* cur_sym = NULL;
int cur_level = 0;

vector< vector<struct Symbol*> > symbolMap;
vector<struct Symbol*> symbolBuffer;
vector< vector<struct Symbol*> > semanticMap;
vector<struct Symbol*> semanticBuffer;

vector<struct Symbol*> arrayInitBuffer;

vector<int> exprCountVector;

string scalar_type_str;
string ret_type_str;

int array_sz = 1;
int eq_cmp = 0;
int actual_sz = 0;
int loop_count = 0;

int error_count = 0;
//============== project 4 ========

struct Symbol* getSymbol(char* symbol_name, int flag, int level){
    ssize_t map_sz = symbolMap.size(), v_sz;
    
    //printf("Get symbol %s\n", symbol_name);
    vector<struct Symbol*> arguments;
    if (flag & IS_FUNCTION){
        //printf("expr_count: %d\n", exprCountVector.back());
        for (int i=exprCountVector.back(); i>0; i--){
            arguments.push_back(semanticBuffer.back());
            semanticBuffer.pop_back();
        }
        exprCountVector.pop_back();
    }
    
    for(int i=map_sz-1; i>=0; i--){
        v_sz = symbolMap[i].size();
        for(int j=0; j<v_sz; j++){
            if( (flag & IS_FUNCTION) && (strcmp(symbol_name, symbolMap[i][j]->name) == 0)){
                ssize_t argu_sz = arguments.size();
                ssize_t k = 0;
                char* tmp, *buf;
                asprintf(&tmp, "%s", symbolMap[i][j]->attribute);
                buf = strtok(tmp, ",");
                while(buf){
                    
                    string s[2] = {string(tmp), string(arguments[k]->type)};
                    int type[2] = {-1, -1};
                    string typeMap[3] = {string("int"), string("float"), string("double")};
                    for(int y = 0; y <2; y++){ 
                        for (int z=0; z<3; z++){
                            int pos = s[y].find(typeMap[z]);
                            if(pos != string::npos){
                                s[y].erase(pos, typeMap[z].length());
                                type[y] = z; 
                            }
                        }
                    }
                    //printf("%s, %s\n", s[0].c_str(), s[1].c_str());
                    if(k >= argu_sz || 
                            ( (type[0] != -1 && type[1] != -1) &&  (type[0] < type[1]) ) ||
                            ( (type[0] != -1 && type[1] != -1)  && (type[0] >= type[1]) &&  (s[0] != s[1]) ) ||
                            ( s[0] != s[1] )  )
                    {   
                        break;
                    }
                    buf = strtok(NULL, ",");
                    k++;
                }
                free(tmp);
                if(k == argu_sz) return symbolMap[i][j];
                else {
                    functionInvokeMissError();
                    return NULL;
                }
            }                
            else if( (flag & IS_ARRAY) && (strcmp(symbol_name, symbolMap[i][j]->name) == 0)){
                char *tmp, *buf;
                int l = 0;
                
                asprintf(&tmp, "%s", symbolMap[i][j]->type);
                buf = tmp;
                buf = strtok(tmp, "[" );
                buf = strtok(NULL, "[");
                while(buf){
                    l++;
                    buf = strtok(NULL, "[");
                }
                free(tmp);
                //printf("name: %s, type: %s, getSym: %d, %d\n", symbolMap[i][j]->name, symbolMap[i][j]->type, l, level);
                if (l == level)
                    return symbolMap[i][j];
            }
            else if(strcmp(symbol_name, symbolMap[i][j]->name) == 0){
                return symbolMap[i][j];
            }
        }
    }
    undefinedError(symbol_name);
    return NULL;
}


struct Symbol* logicalCheck(int is_binary){
    struct Symbol* s1 = semanticBuffer.back();
    semanticBuffer.pop_back();
    
    struct Symbol* s = (struct Symbol*)malloc(sizeof(struct Symbol));
    
    if (is_binary){
        struct Symbol* s2 = semanticBuffer.back();
        semanticBuffer.pop_back();

        if ((strcmp(s1->type, "bool") == 0) && (strcmp(s2->type, "bool") == 0)){
            asprintf(&(s->type), "%s", "bool");
        }else{
            asprintf(&(s->type), "%s", "Error");
            typeMissError();
        }
    }
    else if(strcmp(s1->type, "bool") == 0){
        asprintf(&(s->type), "%s", "bool");
    }
    else{
        asprintf(&(s->type), "%s", "Error");
        typeMissError();
    }

    return s;
}

struct Symbol* arithCheck(int is_binary, int is_relation ){
    struct Symbol* s1 = semanticBuffer.back(), *s2;
    semanticBuffer.pop_back();
    
        
    int t1, t2 = 0;

    if ( strcmp(s1->type, "int") == 0 ) t1 = 0;
    else if ( strcmp(s1->type, "float") == 0 ) t1 = 1;
    else if ( strcmp(s1->type, "double") == 0 ) t1 = 2;
    else t1 = -1;

    if (is_binary){
        s2 = semanticBuffer.back();
        semanticBuffer.pop_back();

        if ( strcmp(s2->type, "int") == 0 ) t2 = 0;
        else if ( strcmp(s2->type, "float") == 0 ) t2 = 1;
        else if ( strcmp(s2->type, "double") == 0 ) t2 = 2;
        else t2 = -1;
    }
    int t = max(t1, t2);
    struct Symbol* s = (struct Symbol*)malloc(sizeof(struct Symbol));

    if( !eq_cmp && (t1 == -1 || t2 == -1)){
        asprintf(&(s->type), "%s", "Error");
        typeMissError();
        return s;
    }
    else if ( eq_cmp && (strcmp(s1->type, "bool")==0) && (strcmp(s2->type, "bool") == 0) ){
        asprintf(&(s->type), "%s", "bool");
    }
    else if( eq_cmp && (t1 == -1 && t2 == -1) && (strcmp(s1->type, "bool") || strcmp(s2->type, "bool") )  ) {
        asprintf(&(s->type), "%s", "Error");
    }
    else if(is_relation) asprintf(&(s->type), "%s", "bool");
    else if (t == 0 ) asprintf(&(s->type), "%s", "int");
    else if (t == 1 ) asprintf(&(s->type), "%s", "float");
    else if (t == 2 ) asprintf(&(s->type), "%s", "double");

    eq_cmp = 0;
    return s;
}

struct Symbol* assignCheck(int flag){
    struct Symbol* s1 = semanticBuffer.back();
    semanticBuffer.pop_back();
    struct Symbol* s2;     
    int t1, t2 = 0;
    if (flag & IS_DECLARE){
        
        if ( strcmp(s1->type, "int") == 0 ) t1 = 0;
        else if ( strcmp(s1->type, "float") == 0 ) t1 = 1;
        else if ( strcmp(s1->type, "double") == 0 ) t1 = 2;
        else t1 = -1;

        if ( scalar_type_str == string("int") ) t2 = 0;
        else if ( scalar_type_str == string("float") ) t2 = 1;
        else if ( scalar_type_str == string("double") ) t2 = 2;
        else t2 = -1;

        if ((t1 == -1 && t2 == -1) && string(s1->type) == scalar_type_str){
            return s1;      
        }
        else if( t2 >= t1 ){
            return s1;
        }
        else{
            typeMissError();
            return NULL;
        }
    }
    else{
        s2 = semanticBuffer.back();
        semanticBuffer.pop_back();
        
        

        if ( strcmp(s1->type, "int") == 0 ) t1 = 0;
        else if ( strcmp(s1->type, "float") == 0 ) t1 = 1;
        else if ( strcmp(s1->type, "double") == 0 ) t1 = 2;
        else t1 = -1;

            
        if ( strcmp(s2->type, "int") == 0 ) t2 = 0;
        else if ( strcmp(s2->type, "float") == 0 ) t2 = 1;
        else if ( strcmp(s2->type, "double") == 0 ) t2 = 2;
        else t2 = -1;
        
        
        struct Symbol* s = (struct Symbol*)malloc(sizeof(struct Symbol));
        
        if (s1->kind && strcmp(s1->kind, "constant") == 0){
            asprintf(&(s->type), "%s", "Error");
            constAssignError();
            return s;
        }
        if (t2 >= t1 && t1 != -1 && t2 != -1){
            if (t2 == 0 ) asprintf(&(s->type), "%s", "int");
            else if (t2 == 1 ) asprintf(&(s->type), "%s", "float");
            else if (t2 == 2 ) asprintf(&(s->type), "%s", "double");
            
            return s;
        }
        if(t1 == -1 || t2 == -1){
            if (strcmp(s1->type, s2->type)){ 
                typeMissError();
                return NULL;
            }
            else asprintf(&(s->type), "%s", s1->type);
            return s;
        }
    } 
}

void isArrayCheck(){
    struct Symbol* buf = semanticBuffer.back();
    semanticBuffer.pop_back();
    
    if(strstr(buf->type, "[")){
        typeMissError();
    }
}

int conditionCheck(){
    struct Symbol* buf = semanticBuffer.back();
    semanticBuffer.pop_back();
    if(strcmp(buf->type, "bool")){
        conditionTypeError();
        return 0;
    }
    return 1;
}

int initArrayCheck(){
    
    vector<struct Symbol*> buf;
    for (int i=0;i<actual_sz;i++){
        buf.push_back(semanticBuffer.back());
        semanticBuffer.pop_back();
    }

    if (actual_sz > array_sz){
        arrayInitError();
        return 0;
    }
    
    int t;
    if ( scalar_type_str == string("int") ) t = 0;
    else if ( scalar_type_str == string("float") ) t = 1;
    else if ( scalar_type_str == string("double") ) t = 2;
    else t = -1;


    for(int i = 0; i < actual_sz; i++){
        if( t==-1 && string(buf[i]->type) != scalar_type_str){
            arrayInitError();
            return 0;
        }
        else {
            int t2;
            if ( strcmp(buf[i]->type, "int") == 0 ) t2 = 0;
            else if ( strcmp(buf[i]->type, "float") == 0 ) t2 = 1;
            else if ( strcmp(buf[i]->type, "double") == 0 ) t2 = 2;
            else t2 = -1;

            if( t2!= -1 && t>=t2 );
            else{
                arrayInitError();
                return 0;
            }

        }
    }

    return 1;    
}

void functionInvokeMissError(){
    error_count = 1;
    printf("########## Error at Line#%d: function parameter not match ##########\n", linenum);
}

void arrayDeclError(){
    error_count = 1;
    printf("########## Error at Line#%d: array declaration index error ##########\n", linenum);
}

void constAssignError(){
    error_count = 1;
    printf("########## Error at Line#%d: constant assignment ##########\n", linenum);
}

void jumpError(){
    error_count = 1;
    printf("########## Error at Line#%d: jump error ##########\n", linenum);
}

void conditionTypeError(){
    error_count = 1;
    printf("########## Error at Line#%d: condition type error ##########\n", linenum);
}
void returnTypeError(){
    error_count = 1;
    printf("########## Error at Line#%d: return type error ##########\n", linenum);
}
void typeMissError(){
    error_count = 1;
    printf("########## Error at Line#%d: expression type error ##########\n", linenum);
}

void undefinedError(char* name){
    error_count = 1;
    printf("########## Error at Line#%d: symbol %s is undefined ##########\n", linenum, name);
}

void arrayInitError(){
    error_count = 1;
    printf("########## Error at Line#%d: array initial error ##########\n", linenum);
}
//============== project 3 ========
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
        bool sym_same;
        bool name_same = (strcmp(top[i]->name, cur_sym->name) == 0);
        if ( name_same && (strcmp(cur_sym->kind, "function") == 0) && (strcmp(top[i]->kind, "function") == 0) )
            sym_same = name_same && (strcmp(top[i]->attribute, cur_sym->attribute) == 0) && (strcmp(top[i]->type, cur_sym->type) == 0);
        if(sym_same && top[i]->is_declare == 1 && cur_sym->is_declare == 0 ){
            top[i]->is_declare == 0;
            return;
        }
        else if(name_same){
            printf("########## Error at Line#%d: symbol %s is redeclared ##########\n", linenum, cur_sym->name);
            error_count = 1;
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
