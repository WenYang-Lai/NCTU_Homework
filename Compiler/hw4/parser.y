%{
#include "symbol_table.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <map>

using namespace std;


extern "C" {
    extern int linenum;
    extern FILE *yyin;
    extern char *yytext;
    extern char buf[256];

    extern int yylex();
    extern int yyerror( char *msg );
};

%}

%union 
{
    int ival;
    float fval;
    char* text;
}

%token  ID
%token  INT_CONST
%token  FLOAT_CONST
%token  SCIENTIFIC
%token  STR_CONST

%token  LE_OP
%token  NE_OP
%token  GE_OP
%token  EQ_OP
%token  AND_OP
%token  OR_OP

%token  READ
%token  BOOLEAN
%token  WHILE
%token  DO
%token  IF
%token  ELSE
%token  TRUE
%token  FALSE
%token  FOR
%token  INT
%token  PRINT
%token  BOOL
%token  VOID
%token  FLOAT
%token  DOUBLE
%token  STRING
%token  CONTINUE
%token  BREAK
%token  RETURN
%token  CONST

%token  L_PAREN
%token  R_PAREN
%token  COMMA
%token  SEMICOLON
%token  ML_BRACE
%token  MR_BRACE
%token  L_BRACE
%token  R_BRACE
%token  ADD_OP
%token  SUB_OP
%token  MUL_OP
%token  DIV_OP
%token  MOD_OP
%token  ASSIGN_OP
%token  LT_OP
%token  GT_OP
%token  NOT_OP

%type<text> ID VOID INT FLOAT STRING DOUBLE BOOL scalar_type parameter_list array_decl dim sign_literal_const literal_const STR_CONST INT_CONST FLOAT_CONST SCIENTIFIC TRUE FALSE dimension

/*  Program 
    Function 
    Array 
    Const 
    IF 
    ELSE 
    RETURN 
    FOR 
    WHILE
*/
%start program


%%

program : { symbolMap.push_back(vector<struct Symbol*>()); } decl_list funct_def decl_and_def_list { decLevel(); } 
        ;

decl_list : decl_list var_decl
          | decl_list const_decl
          | decl_list funct_decl
          |
          ;


decl_and_def_list : decl_and_def_list var_decl
                  | decl_and_def_list const_decl
                  | decl_and_def_list funct_decl
                  | decl_and_def_list funct_def
                  | 
                  ;

funct_def : scalar_type ID  L_PAREN  R_PAREN {
            symbolBuffer.clear();
            createNewSym(); 
            cur_sym->name = $2;
            cur_sym->type = $1;
            ret_type_str = string($1);
            asprintf(&(cur_sym->kind), "%s", "function");
            asprintf(&(cur_sym->attribute), "%s", "");
            insertSymMap();
          }
compound_statement 
          | scalar_type ID L_PAREN parameter_list R_PAREN { 
            createNewSym(); 
            cur_sym->name = $2;
            cur_sym->type = $1;
            ret_type_str = string($1);
            asprintf(&(cur_sym->kind), "%s", "function");
            cur_sym->attribute = $4;
            insertSymMap();
          }
 compound_statement 
          | VOID ID L_PAREN R_PAREN {  
            createNewSym(); 
            cur_sym->name = $2;
            cur_sym->type = $1;
            asprintf(&(cur_sym->kind), "%s", "function");
            ret_type_str = string($1);
            asprintf(&(cur_sym->attribute), "%s", "");
            insertSymMap();
          }
compound_statement 
        | VOID ID L_PAREN  parameter_list R_PAREN { 
            createNewSym(); 
            cur_sym->name = $2;
            cur_sym->type = $1;
            ret_type_str = string($1);
            asprintf(&(cur_sym->kind), "%s", "function");
            cur_sym->attribute = $4;
            insertSymMap();
          }
        compound_statement     
        ;

funct_decl : scalar_type  ID L_PAREN  R_PAREN { 
            createNewSym();
            cur_sym->is_declare = 1;
            cur_sym->name = $2;
            cur_sym->type = $1;
            ret_type_str = string($1);
            asprintf(&(cur_sym->attribute), "%s", "");
            asprintf(&(cur_sym->kind), "%s", "function");
            insertSymMap();
          }
SEMICOLON  {   
            symbolBuffer.clear();
            }


           | scalar_type ID L_PAREN parameter_list R_PAREN {
            symbolBuffer.clear();
            createNewSym(); 
            cur_sym->is_declare = 1;
            cur_sym->name = $2;
            cur_sym->type = $1;
            ret_type_str = string($1);
            asprintf(&(cur_sym->kind), "%s", "function");
            cur_sym->attribute = $4;
            insertSymMap();
          }
SEMICOLON  {   
            symbolBuffer.clear();
            }

           | VOID ID L_PAREN R_PAREN { 
            createNewSym(); 
            cur_sym->is_declare = 1;
            cur_sym->name = $2;
            cur_sym->type = $1;
            ret_type_str = string($1);
            asprintf(&(cur_sym->kind), "%s", "function");
            asprintf(&(cur_sym->attribute), "%s", "");
            insertSymMap();
          }
SEMICOLON  {   
            symbolBuffer.clear();
            }

        | VOID ID L_PAREN parameter_list R_PAREN { 
            createNewSym(); 
            cur_sym->is_declare = 1;
            cur_sym->name = $2;
            cur_sym->type = $1;
            ret_type_str = string($1);
            asprintf(&(cur_sym->kind), "%s", "function");
            cur_sym->attribute = $4;
            insertSymMap();
          }
    SEMICOLON {   
            symbolBuffer.clear();
            }
        ;

parameter_list : parameter_list COMMA scalar_type ID {
                createNewSym(); 
                cur_sym->name = $4;
                cur_sym->type = $3;
                asprintf(&(cur_sym->kind), "%s", "parameter");
                symbolBuffer.push_back(cur_sym);
                asprintf(&($$), "%s,%s", $1, cur_sym->type);
               }
               | parameter_list COMMA scalar_type array_decl
               {
                char* tmp;
                createNewSym();
                tmp = strtok($4, " ");
                asprintf(&(cur_sym->name), "%s", tmp);
                tmp = strtok(NULL, " ");
                asprintf(&(cur_sym->type), "%s%s", $3, tmp);
                asprintf(&(cur_sym->kind), "%s", "parameter");
                symbolBuffer.push_back(cur_sym);
                asprintf(&($$), "%s,%s", $1, cur_sym->type);
                if (array_sz == 0) arrayDeclError();
                array_sz = 1;
               }
               | scalar_type array_decl { 
                char* tmp;
                createNewSym();
                tmp = strtok($2, " ");
                asprintf(&(cur_sym->name), "%s", tmp);
                tmp = strtok(NULL, " ");
                asprintf(&(cur_sym->type), "%s%s", $1, tmp);
                asprintf(&(cur_sym->kind), "%s", "parameter");
                symbolBuffer.push_back(cur_sym);
                $$ = cur_sym->type;
                if (array_sz == 0) arrayDeclError();
                array_sz = 1;
              }

               | scalar_type ID { 
                createNewSym(); 
                cur_sym->name = $2;
                cur_sym->type = $1;
                asprintf(&(cur_sym->kind), "%s", "parameter");
                symbolBuffer.push_back(cur_sym);
                $$ = $1;
              }

               ;

var_decl : scalar_type identifier_list SEMICOLON {
            size_t sz = symbolBuffer.size();
            char* tmp;
            for (int i=0;i<sz;i++){
               cur_sym = symbolBuffer[i];
               asprintf(&tmp, "%s%s", $1, cur_sym->type ? cur_sym->type : "");
               asprintf(&(cur_sym->kind), "%s", "variable");
               free(cur_sym->type);
               cur_sym->type = tmp;
               insertSymMap();
            }
            symbolBuffer.clear();
         }
         ;

identifier_list : identifier_list COMMA ID {
                    createNewSym();
                    asprintf(&(cur_sym->name), "%s", $3);
                    symbolBuffer.push_back(cur_sym);
                }

                | identifier_list COMMA ID ASSIGN_OP logical_expression {
                    if (assignCheck(IS_DECLARE)){
                        createNewSym();
                        asprintf(&(cur_sym->name), "%s", $3);
                        symbolBuffer.push_back(cur_sym);
                    }
                }

                | identifier_list COMMA array_decl ASSIGN_OP initial_array {
                    if(initArrayCheck()){
                        char* tmp;
                        createNewSym();
                        tmp = strtok($3, " ");
                        asprintf(&(cur_sym->name), "%s", tmp);
                        tmp = strtok(NULL, " ");
                        asprintf(&(cur_sym->type), "%s", tmp);
                        symbolBuffer.push_back(cur_sym);
                    }
                    if (array_sz == 0) arrayDeclError();
                    array_sz = 1;
                }
                | identifier_list COMMA array_decl {
                    char* tmp;
                    createNewSym();
                    tmp = strtok($3, " ");
                    asprintf(&(cur_sym->name), "%s", tmp);
                    tmp = strtok(NULL, " ");
                    asprintf(&(cur_sym->type), "%s", tmp);
                    symbolBuffer.push_back(cur_sym);
                    if (array_sz == 0) arrayDeclError();
                    array_sz = 1;
                }

                | array_decl ASSIGN_OP initial_array {
                    if(initArrayCheck()){
                        char* tmp;
                        createNewSym();
                        tmp = strtok($1, " ");
                        asprintf(&(cur_sym->name), "%s", tmp);
                        tmp = strtok(NULL, " ");
                        asprintf(&(cur_sym->type), "%s", tmp);
                        symbolBuffer.push_back(cur_sym);
                    }
                    if (array_sz == 0) arrayDeclError();
                    array_sz = 1;
                }
                | array_decl {
                    char* tmp;
                    createNewSym();
                    tmp = strtok($1, " ");
                    asprintf(&(cur_sym->name), "%s", tmp);
                    tmp = strtok(NULL, " ");
                    asprintf(&(cur_sym->type), "%s", tmp);
                    symbolBuffer.push_back(cur_sym);
                    if (array_sz == 0) arrayDeclError();
                    array_sz = 1;
                }
                | ID ASSIGN_OP logical_expression {
                    if(assignCheck(IS_DECLARE)){
                        createNewSym();
                        asprintf(&(cur_sym->name), "%s", $1);
                        symbolBuffer.push_back(cur_sym);

                    }
                }
                | ID {
                    createNewSym();
                    asprintf(&(cur_sym->name), "%s", $1);
                    symbolBuffer.push_back(cur_sym);
                }
                ;

initial_array : {actual_sz = 0;} L_BRACE literal_list R_BRACE
              ;

literal_list : literal_list COMMA logical_expression {actual_sz +=1;}
             | logical_expression { actual_sz +=1; }
             | 
             ;

const_decl : CONST scalar_type const_list SEMICOLON {
            size_t sz = symbolBuffer.size();
            for (int i=0;i<sz;i++){
               cur_sym = symbolBuffer[i];
               asprintf(&(cur_sym->type), "%s", $2);
               asprintf(&(cur_sym->kind), "%s", "constant");
               insertSymMap();
            }
            symbolBuffer.clear();

           };

const_list : const_list COMMA ID ASSIGN_OP sign_literal_const {
                if(assignCheck(IS_DECLARE)){
                    createNewSym();
                    asprintf(&(cur_sym->name), "%s", $3);
                    asprintf(&(cur_sym->attribute), "%s", $5);
                    symbolBuffer.push_back(cur_sym);
                }
           }
           | ID ASSIGN_OP sign_literal_const {
            if(assignCheck(IS_DECLARE)){
                createNewSym();
                asprintf(&(cur_sym->name), "%s", $1);
                asprintf(&(cur_sym->attribute), "%s", $3);
                symbolBuffer.push_back(cur_sym);
            }
           }
           ;

array_decl : ID dim {
            asprintf(&($$), "%s %s", $1, $2);
           }
           ;

dim : dim ML_BRACE INT_CONST MR_BRACE {
        asprintf(&($$), "%s[%s]", $1, $3);
        array_sz *= atoi($3);
    }
    | ML_BRACE INT_CONST MR_BRACE{
        asprintf(&($$), "[%s]", $2);
        array_sz *= atoi($2);
    }

    ;

compound_statement : {  
                        incLevel(); 
                        size_t sz = symbolBuffer.size();
                        for (int i=0;i<sz;i++){
                            cur_sym = symbolBuffer[i];
                            asprintf(&(cur_sym->kind), "%s", "parameter");
                            insertSymMap();
                        }
                        symbolBuffer.clear();

                   } L_BRACE var_const_stmt_list R_BRACE {
                        decLevel();
                    }
                   ;

var_const_stmt_list : var_const_stmt_list statement 
                    | var_const_stmt_list var_decl
                    | var_const_stmt_list const_decl
                    |
                    ;

statement : compound_statement
          | simple_statement
          | conditional_statement
          | while_statement
          | for_statement
          | function_invoke_statement SEMICOLON
          | jump_statement
          ;     

simple_statement : variable_reference ASSIGN_OP logical_expression SEMICOLON {
                    assignCheck();
                 }
                 | PRINT logical_expression SEMICOLON{
                    isArrayCheck();
                 }
                 | READ variable_reference SEMICOLON {
                   isArrayCheck(); 
                 }
                 ;

conditional_statement : IF L_PAREN logical_expression  R_PAREN compound_statement {conditionCheck();} 
                      | IF L_PAREN logical_expression  R_PAREN compound_statement {conditionCheck();} 
                        ELSE compound_statement
                      ;
while_statement : WHILE L_PAREN logical_expression  R_PAREN {conditionCheck(); loop_count++;}compound_statement
                    | DO {
              loop_count++;
              } 
compound_statement WHILE L_PAREN logical_expression R_PAREN {conditionCheck();}SEMICOLON { loop_count--; }
                ;

for_statement : FOR L_PAREN initial_expression_list SEMICOLON control_expression_list SEMICOLON increment_expression_list R_PAREN {
              loop_count++;
              semanticBuffer.pop_back();
              semanticBuffer.pop_back();
              semanticBuffer.pop_back();
              } 
                    compound_statement
              ;

initial_expression_list : initial_expression
                        | {
                            struct Symbol* sym = (struct Symbol*)malloc(sizeof(struct Symbol));
                            semanticBuffer.push_back(sym);
                        }
                        ;

initial_expression : initial_expression COMMA variable_reference ASSIGN_OP logical_expression{
                    struct Symbol* sym = assignCheck();
                    semanticBuffer.pop_back();
                    semanticBuffer.push_back(sym);
                   }

| initial_expression COMMA logical_expression{
                    semanticBuffer.pop_back();
                   }
                   | logical_expression
                   | variable_reference ASSIGN_OP logical_expression {
                    semanticBuffer.push_back(assignCheck());
                   }

control_expression_list : control_expression
                        |{
                            struct Symbol* sym = (struct Symbol*)malloc(sizeof(struct Symbol));
                            semanticBuffer.push_back(sym);
                        }
                        ;

control_expression : control_expression COMMA variable_reference ASSIGN_OP logical_expression{
                    struct Symbol* sym = assignCheck();
                    semanticBuffer.pop_back();
                    semanticBuffer.push_back(sym);
                   }
| control_expression COMMA logical_expression{
                    semanticBuffer.pop_back();
                   }
                   | logical_expression
                   | variable_reference ASSIGN_OP logical_expression{
                    semanticBuffer.push_back(assignCheck());
                   }

                   ;

increment_expression_list : increment_expression 
                          |{
                            struct Symbol* sym = (struct Symbol*)malloc(sizeof(struct Symbol));
                            semanticBuffer.push_back(sym);
                        }

                          ;

increment_expression : increment_expression COMMA variable_reference ASSIGN_OP logical_expression{
                    struct Symbol* sym = assignCheck();
                    semanticBuffer.pop_back();
                    semanticBuffer.push_back(sym);
                   }

| increment_expression COMMA logical_expression{
                        semanticBuffer.pop_back();
                     }
                     | logical_expression
                     | variable_reference ASSIGN_OP logical_expression{
                    semanticBuffer.push_back(assignCheck());
                   }

                     ;

function_invoke_statement : ID L_PAREN { exprCountVector.push_back(0); } logical_expression_list R_PAREN {
                            struct Symbol* func_sym = getSymbol($1,1, 0);
                            if(!func_sym){
                                func_sym = (struct Symbol*)malloc(sizeof(struct Symbol));
                                asprintf(&(func_sym->type), "%s", "Error");
                            }
                            semanticBuffer.push_back(func_sym);
                          }
                          | ID L_PAREN { exprCountVector.push_back(0); }R_PAREN {
                            struct Symbol* func_sym = getSymbol($1,1,0);
                            if(!func_sym){
                                func_sym = (struct Symbol*)malloc(sizeof(struct Symbol));
                                asprintf(&(func_sym->type), "%s", "Error");
                            }
                            semanticBuffer.push_back(func_sym);
                            
                            }
                          ;

jump_statement : CONTINUE SEMICOLON{
               if (!loop_count){
                    jumpError();
                }
               }
               | BREAK SEMICOLON{
               if (!loop_count){
                    jumpError();
                }
               }

               | RETURN logical_expression SEMICOLON {
                struct Symbol* sym = semanticBuffer.back();
                semanticBuffer.pop_back();
                int t[2] = {0};
                string s[2] = {string(sym->type), ret_type_str};
                
                for (int i = 0; i <2; i++){
                    if (s[i] == string("int")) t[i] = 0;
                    else if (s[i] == string("float")) t[i] = 1;
                    else if (s[i] == string("double")) t[i] = 2;
                    else t[i] = -1;
                }
                
                if (t[0] != -1 && t[1] != -1){
                    if (t[1] >= t[0]);
                    else returnTypeError();
                }
                else if (string(sym->type) != ret_type_str){
                    returnTypeError();
                }
               }
               ;

variable_reference : array_list
                   | ID {
                    struct Symbol* sym = getSymbol($1, 0, 0);
                    if (!sym){
                        sym = (struct Symbol*)malloc(sizeof(struct Symbol));
                        asprintf(&sym->type, "%s", "Error");
                    }
                    semanticBuffer.push_back(sym); 
                   }
                   ;


logical_expression : logical_expression OR_OP logical_term { 
                       semanticBuffer.push_back(logicalCheck(1)); 
                   }
                   | logical_term
                   ;

logical_term : logical_term AND_OP logical_factor{ 
                    semanticBuffer.push_back(logicalCheck(1)); 
                }
             | logical_factor
             ;

logical_factor : NOT_OP logical_factor { 
                    semanticBuffer.push_back(logicalCheck(0)); 
                }

               | relation_expression
               ;

relation_expression : relation_expression relation_operator arithmetic_expression {
                        semanticBuffer.push_back(arithCheck(1, 1));
                    }
                    | arithmetic_expression
                    ;

relation_operator : LT_OP   { eq_cmp = 0; }
                  | LE_OP   { eq_cmp = 0; }
                  | EQ_OP   { eq_cmp = 1; }
                  | GE_OP   { eq_cmp = 0; }
                  | GT_OP   { eq_cmp = 0; }
                  | NE_OP   { eq_cmp = 1; }
                  ;

arithmetic_expression : arithmetic_expression ADD_OP term{
                        semanticBuffer.push_back(arithCheck(1, 0));
                      }
                      | arithmetic_expression SUB_OP term{
                        semanticBuffer.push_back(arithCheck(1, 0));
                      }
                      | term
                      ;

term : term MUL_OP factor{
                        semanticBuffer.push_back(arithCheck(1, 0));
                      }

     | term DIV_OP factor{
                        semanticBuffer.push_back(arithCheck(1, 0));
                      }

     | term MOD_OP factor{
                        semanticBuffer.push_back(arithCheck(1, 0));
                      }

     | factor
     ;

factor : sign_literal_const
       | element
       ;

element : SUB_OP element{
                        semanticBuffer.push_back(arithCheck(0, 0));
                      }

        | variable_reference
        | L_PAREN logical_expression R_PAREN 
        | function_invoke_statement
        ;



logical_expression_list :  logical_expression_list COMMA logical_expression {
                        int i = exprCountVector.size();
                        if(i) exprCountVector[i-1]++;
                        }
                        |  logical_expression {
                            int i = exprCountVector.size();
                            if(i) exprCountVector[i-1]++;                        
                            }

                        ;

array_list : ID dimension {
            struct Symbol* sym = getSymbol($1, IS_ARRAY, atoi($2));
            struct Symbol* tmp = (struct Symbol*)malloc(sizeof(struct Symbol));
            if(!sym){
                asprintf(&(tmp->type), "%s", "Error");
            }
            else{
                char* str, *buf;
                asprintf(&str, "%s", sym->type);
                buf = strtok(str, "[");
                asprintf(&(tmp->type), "%s", buf);
                sym = tmp;
                free(str);
            }
            semanticBuffer.push_back(tmp);
           }
           ;

dimension : dimension ML_BRACE logical_expression MR_BRACE 
          {
            struct Symbol* sym = semanticBuffer.back();
            semanticBuffer.pop_back();
            asprintf(&($$), "%d", atoi($1)+1);
            if(strcmp(sym->type, "int")){
                typeMissError();
            }
          }
          | ML_BRACE logical_expression MR_BRACE
          {
            struct Symbol* sym = semanticBuffer.back();
            semanticBuffer.pop_back();
            if(strcmp(sym->type, "int")){
                typeMissError();
            }
            asprintf(&($$), "%s", "1");
          }
          ;



scalar_type : INT { $$ = $1; scalar_type_str = string($1); }
            | DOUBLE { $$ = $1;scalar_type_str = string($1); }
            | STRING { $$ = $1;scalar_type_str = string($1); } 
            | BOOL { $$ = $1;scalar_type_str = string($1); }
            | FLOAT { $$ = $1;scalar_type_str = string($1); }
            ;

sign_literal_const : SUB_OP sign_literal_const { $$ = $2; semanticBuffer.push_back(arithCheck(0, 0)); } 
                   | literal_const 
                   ;

literal_const : INT_CONST{ 
                struct Symbol* sym = (struct Symbol*)malloc(sizeof(struct Symbol));
                asprintf(&(sym->type), "%s", "int");
                semanticBuffer.push_back(sym);
              }
              | FLOAT_CONST{ 
                struct Symbol* sym = (struct Symbol*)malloc(sizeof(struct Symbol));
                asprintf(&(sym->type), "%s", "float");
                semanticBuffer.push_back(sym);
              }
              | SCIENTIFIC { 
                struct Symbol* sym = (struct Symbol*)malloc(sizeof(struct Symbol));
                asprintf(&(sym->type), "%s", "double");
                semanticBuffer.push_back(sym);
              }
              | STR_CONST { 
                struct Symbol* sym = (struct Symbol*)malloc(sizeof(struct Symbol));
                asprintf(&(sym->type), "%s", "string");
                semanticBuffer.push_back(sym);
              }
              | TRUE { 
                struct Symbol* sym = (struct Symbol*)malloc(sizeof(struct Symbol));
                asprintf(&(sym->type), "%s", "bool");
                semanticBuffer.push_back(sym);
              }
              | FALSE { 
                struct Symbol* sym = (struct Symbol*)malloc(sizeof(struct Symbol));
                asprintf(&(sym->type), "%s", "bool");
                semanticBuffer.push_back(sym);
              }
              ;

%%

int yyerror( char *msg )
{
    fprintf( stderr, "\n|--------------------------------------------------------------------------\n" );
    fprintf( stderr, "| Error found in Line #%d: %s\n", linenum, buf );
    fprintf( stderr, "|\n" );
    fprintf( stderr, "| Unmatched token: %s\n", yytext );
    fprintf( stderr, "|--------------------------------------------------------------------------\n" );
    exit(-1);
    //  fprintf( stderr, "%s\t%d\t%s\t%s\n", "Error found in Line ", linenum, "next token: ", yytext );
}


