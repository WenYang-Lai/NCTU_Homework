%{
#include "symbol_table.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <map>

using namespace std;
extern "C"{
    extern int linenum;
    extern FILE *yyin;
    extern char *yytext;
    extern char buf[256];

    extern int yylex();
    extern int yyerror( char *msg );
}

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

%type<text> ID VOID INT FLOAT STRING DOUBLE BOOL scalar_type parameter_list array_decl dim sign_literal_const literal_const STR_CONST INT_CONST FLOAT_CONST SCIENTIFIC TRUE FALSE 

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

funct_def : scalar_type ID L_PAREN R_PAREN compound_statement {
            symbolBuffer.clear();
            createNewSym(); 
            cur_sym->name = $2;
            cur_sym->type = $1;
            asprintf(&(cur_sym->kind), "%s", "function");
            asprintf(&(cur_sym->attribute), "%s", "");
            insertSymMap();
          }
          | scalar_type ID L_PAREN parameter_list R_PAREN  compound_statement { 
            symbolBuffer.clear();
            createNewSym(); 
            cur_sym->name = $2;
            cur_sym->type = $1;
            asprintf(&(cur_sym->kind), "%s", "function");
            cur_sym->attribute = $4;
            insertSymMap();
          }

          | VOID ID L_PAREN R_PAREN compound_statement {  
            symbolBuffer.clear();
            createNewSym(); 
            cur_sym->name = $2;
            cur_sym->type = $1;
            asprintf(&(cur_sym->kind), "%s", "function");
            asprintf(&(cur_sym->attribute), "%s", "");
            insertSymMap();
          }
          | VOID ID L_PAREN  parameter_list R_PAREN compound_statement { 
            symbolBuffer.clear();
            createNewSym(); 
            cur_sym->name = $2;
            cur_sym->type = $1;
            asprintf(&(cur_sym->kind), "%s", "function");
            cur_sym->attribute = $4;
            insertSymMap();
          }
          ;

funct_decl : scalar_type ID L_PAREN R_PAREN SEMICOLON { 
            symbolBuffer.clear();
            createNewSym(); 
            cur_sym->is_declare = 1;
            cur_sym->name = $2;
            cur_sym->type = $1;
            asprintf(&(cur_sym->kind), "%s", "function");
            insertSymMap();
          }

           | scalar_type ID L_PAREN parameter_list R_PAREN SEMICOLON { 
            symbolBuffer.clear();
            createNewSym(); 
            cur_sym->is_declare = 1;
            cur_sym->name = $2;
            cur_sym->type = $1;
            asprintf(&(cur_sym->kind), "%s", "function");
            cur_sym->attribute = $4;
            insertSymMap();
          }

           | VOID ID L_PAREN R_PAREN SEMICOLON { 
            symbolBuffer.clear();
            createNewSym(); 
            cur_sym->is_declare = 1;
            cur_sym->name = $2;
            cur_sym->type = $1;
            asprintf(&(cur_sym->kind), "%s", "function");
            asprintf(&(cur_sym->attribute), "%s", "");
            insertSymMap();
          }
           | VOID ID L_PAREN parameter_list R_PAREN SEMICOLON { 
            symbolBuffer.clear();
            createNewSym(); 
            cur_sym->is_declare = 1;
            cur_sym->name = $2;
            cur_sym->type = $1;
            asprintf(&(cur_sym->kind), "%s", "function");
            cur_sym->attribute = $4;
            insertSymMap();
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
                    createNewSym();
                    asprintf(&(cur_sym->name), "%s", $3);
                    symbolBuffer.push_back(cur_sym);
                }

                | identifier_list COMMA array_decl ASSIGN_OP initial_array {
                    char* tmp;
                    createNewSym();
                    tmp = strtok($3, " ");
                    asprintf(&(cur_sym->name), "%s", tmp);
                    tmp = strtok(NULL, " ");
                    asprintf(&(cur_sym->type), "%s", tmp);
                    symbolBuffer.push_back(cur_sym);
                }

                | identifier_list COMMA array_decl {
                    char* tmp;
                    createNewSym();
                    tmp = strtok($3, " ");
                    asprintf(&(cur_sym->name), "%s", tmp);
                    tmp = strtok(NULL, " ");
                    asprintf(&(cur_sym->type), "%s", tmp);
                    symbolBuffer.push_back(cur_sym);
                }

                | array_decl ASSIGN_OP initial_array {
                    char* tmp;
                    createNewSym();
                    tmp = strtok($1, " ");
                    asprintf(&(cur_sym->name), "%s", tmp);
                    tmp = strtok(NULL, " ");
                    asprintf(&(cur_sym->type), "%s", tmp);
                    symbolBuffer.push_back(cur_sym);
                }
                | array_decl {
                    char* tmp;
                    createNewSym();
                    tmp = strtok($1, " ");
                    asprintf(&(cur_sym->name), "%s", tmp);
                    tmp = strtok(NULL, " ");
                    asprintf(&(cur_sym->type), "%s", tmp);
                    symbolBuffer.push_back(cur_sym);
                }
                | ID ASSIGN_OP logical_expression {
                    createNewSym();
                    asprintf(&(cur_sym->name), "%s", $1);
                    symbolBuffer.push_back(cur_sym);
                }
                | ID {
                    createNewSym();
                    asprintf(&(cur_sym->name), "%s", $1);
                    symbolBuffer.push_back(cur_sym);
                }
                ;

initial_array : L_BRACE literal_list R_BRACE
              ;

literal_list : literal_list COMMA logical_expression
             | logical_expression
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
                createNewSym();
                asprintf(&(cur_sym->name), "%s", $3);
                asprintf(&(cur_sym->attribute), "%s", $5);
                symbolBuffer.push_back(cur_sym);
           }
           | ID ASSIGN_OP sign_literal_const {
                createNewSym();
                asprintf(&(cur_sym->name), "%s", $1);
                asprintf(&(cur_sym->attribute), "%s", $3);
                symbolBuffer.push_back(cur_sym);

           }
           ;

array_decl : ID dim {
            asprintf(&($$), "%s %s", $1, $2);
           }
           ;

dim : dim ML_BRACE INT_CONST MR_BRACE {
        asprintf(&($$), "%s[%s]", $1, $3);
    }
    | ML_BRACE INT_CONST MR_BRACE{
        asprintf(&($$), "[%s]", $2);
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
          | function_invoke_statement
          | jump_statement
          ;     

simple_statement : variable_reference ASSIGN_OP logical_expression SEMICOLON
                 | PRINT logical_expression SEMICOLON
                 | READ variable_reference SEMICOLON
                 ;

conditional_statement : IF L_PAREN logical_expression R_PAREN compound_statement 
                      | IF L_PAREN logical_expression R_PAREN compound_statement
                        ELSE compound_statement
                      ;
while_statement : WHILE L_PAREN logical_expression R_PAREN
                    compound_statement
                    | DO compound_statement WHILE L_PAREN logical_expression R_PAREN SEMICOLON
                ;

for_statement : FOR L_PAREN initial_expression_list SEMICOLON control_expression_list SEMICOLON increment_expression_list R_PAREN 
                    compound_statement
              ;

initial_expression_list : initial_expression
                        |
                        ;

initial_expression : initial_expression COMMA variable_reference ASSIGN_OP logical_expression
                   | initial_expression COMMA logical_expression
                   | logical_expression
                   | variable_reference ASSIGN_OP logical_expression

control_expression_list : control_expression
                        |
                        ;

control_expression : control_expression COMMA variable_reference ASSIGN_OP logical_expression
                   | control_expression COMMA logical_expression
                   | logical_expression
                   | variable_reference ASSIGN_OP logical_expression
                   ;

increment_expression_list : increment_expression 
                          |
                          ;

increment_expression : increment_expression COMMA variable_reference ASSIGN_OP logical_expression
                     | increment_expression COMMA logical_expression
                     | logical_expression
                     | variable_reference ASSIGN_OP logical_expression
                     ;

function_invoke_statement : ID L_PAREN logical_expression_list R_PAREN SEMICOLON
                          | ID L_PAREN R_PAREN SEMICOLON
                          ;

jump_statement : CONTINUE SEMICOLON
               | BREAK SEMICOLON
               | RETURN logical_expression SEMICOLON
               ;

variable_reference : array_list
                   | ID
                   ;


logical_expression : logical_expression OR_OP logical_term
                   | logical_term
                   ;

logical_term : logical_term AND_OP logical_factor
             | logical_factor
             ;

logical_factor : NOT_OP logical_factor
               | relation_expression
               ;

relation_expression : relation_expression relation_operator arithmetic_expression
                    | arithmetic_expression
                    ;

relation_operator : LT_OP
                  | LE_OP
                  | EQ_OP
                  | GE_OP
                  | GT_OP
                  | NE_OP
                  ;

arithmetic_expression : arithmetic_expression ADD_OP term
                      | arithmetic_expression SUB_OP term
                      | term
                      ;

term : term MUL_OP factor
     | term DIV_OP factor
     | term MOD_OP factor
     | factor
     ;

factor : sign_literal_const
       | element
       ;

element : SUB_OP element
        | variable_reference
        | L_PAREN logical_expression R_PAREN
        | ID L_PAREN logical_expression_list R_PAREN
        | ID L_PAREN R_PAREN
        ;



logical_expression_list : logical_expression_list COMMA logical_expression
                        | logical_expression
                        ;

array_list : ID dimension
           ;

dimension : dimension ML_BRACE logical_expression MR_BRACE         
          | ML_BRACE logical_expression MR_BRACE
          ;



scalar_type : INT { $$ = $1; }
            | DOUBLE { $$ = $1; }
            | STRING { $$ = $1; } 
            | BOOL { $$ = $1; }
            | FLOAT { $$ = $1; }
            ;

sign_literal_const : SUB_OP sign_literal_const { $$ = $2; }
                   | literal_const 
                   ;

literal_const : INT_CONST
              | FLOAT_CONST
              | SCIENTIFIC
              | STR_CONST
              | TRUE
              | FALSE
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


