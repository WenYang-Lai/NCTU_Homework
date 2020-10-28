%{
#include <stdio.h>
#include <stdlib.h>

extern int linenum;             /* declared in lex.l */
extern FILE *yyin;              /* declared by lex */
extern char *yytext;            /* declared by lex */
extern char buf[256];           /* declared in lex.l */
%}

%token ID           /* identifier */
%token NUM FLOATING SCI TRUE FALSE STR_CONST  /* imeediate values */
%token CONST    /* type attributes */

%token INT CHAR FLOAT DOUBLE VOID BOOLEAN BOOL STRING/* type keyword */
%token DO ELSE IF WHILE FOR CONTINUE BREAK RETURN /* statment keywords */
%token READ PRINT /* function name */

%token LESS LESS_EQUAL EQUAL GREATER GREATER_EQUAL NOT_EQUAL AND OR /* operators */

%left '+' '-'
%left '*' '/'
%right '=' '!'

%%


/* definition and declaration */
program : decl_and_def_list
        ;

decl_and_def_list	: def_list
                    | declaration decl_and_def_list
			        ;

accept_decl_and_def_list : def_list
                         | declaration accept_decl_and_def_list
                         | %empty
                         ;


def_list : definition accept_decl_and_def_list
         ;

declaration : const_decl
            | var_decl
            | func_decl
            ;

definition : type identifier '(' argument_list ')' '{' compound '}'
           ;

compound : compound declaration
         | compound statement
         | %empty
         ;

statement : variable_reference '=' expr ';' /* Assignment */
          | PRINT print_argu ';'          
          | READ variable_reference ';'
          | IF '(' expr ')' '{' compound '}' orelse 
          | WHILE '(' expr ')' '{' compound '}'
          | DO '{' compound '}' WHILE '(' expr ')' ';'
          | FOR '(' assign_expr ';' assign_expr ';' assign_expr ')' '{' compound '}'
          | RETURN expr ';'
          | BREAK ';'
          | CONTINUE ';'
          | function_call ';' /* function call */
          ;

function_call : identifier '(' expr_list ')' 
              ;

orelse : ELSE '{' compound '}'
       | %empty
       ;

print_argu : expr 
           ;


/* list */
argument_list : non_empty_argu_list
              | %empty
              ;

expr_list : non_empty_expr_list
          | %empty
          ;

non_empty_expr_list : non_empty_expr_list ',' expr
                    | expr
                    ;

non_empty_argu_list : non_empty_argu_list ',' argu_decl
                    | argu_decl
                    ;

identifier_list : non_empty_id_list
                ;

non_empty_id_list : non_empty_id_list ',' identifier
                  | non_empty_id_list ',' identifier_with_init
                  | non_empty_id_list ',' array_identifier
                  | non_empty_id_list ',' array_identifier_with_init
                  | identifier_with_init
                  | array_identifier_with_init
                  | identifier
                  | array_identifier
                  ;

const_list : non_empty_const_list
           ;

non_empty_const_list : non_empty_const_list ',' const_identifier_with_init
                     | const_identifier_with_init
                     ;


const_decl : CONST type const_list ';' 
           ;

var_decl : type identifier_list ';'
         ;

func_decl : type identifier '(' argument_list ')' ';'
          ;

argu_decl : type identifier 
          | type array_identifier
          ;

type : INT
     | CHAR
     | FLOAT
     | DOUBLE
     | VOID
     | BOOLEAN
     | BOOL
     | STRING
     ; 

assign_expr : identifier '=' expr
             | array_decl '=' expr
             | expr
             ;

expr : expr binary_operator expr_T
     | unary_operator expr_T
     | expr_T
     ; 

expr_T : expr_F
       ;

expr_F : '(' expr ')'
       | NUM 
       | SCI
       | FLOATING
       | identifier or_func_call_array_idx
       | STR_CONST
       | TRUE
       | FALSE
       ;


or_func_call_array_idx : '(' expr_list ')'
                       | array_idx 
                       | %empty
                       ;

unary_operator : '-'
               | '!'
               ;


binary_operator : '+' 
               | '-'
               | '/' 
               | '*' 
               | '%'
               | GREATER 
               | GREATER_EQUAL
               | EQUAL 
               | NOT_EQUAL
               | LESS 
               | LESS_EQUAL 
               | AND 
               | OR
               ;

array_decl : array_decl '[' integer_literal ']'
           | '[' integer_literal']'
           ;

array_idx : array_idx '[' expr ']'
          | '[' expr ']'
          ;

integer_literal : '-' NUM
                | NUM
                ;

variable_reference : identifier or_array_idx
                   ;

or_array_idx : array_idx
             | %empty
             ;

identifier : ID
    	   ;	

identifier_with_init : identifier '=' expr
                     ;

const_identifier_with_init : identifier '=' const_init_value
                           ;

const_init_value : NUM 
                 | SCI
                 | FLOATING
                 | STR_CONST
                 | TRUE
                 | FALSE
                 ;


array_identifier : identifier array_decl
                 ;

array_identifier_with_init : identifier array_decl '=' '{' expr_list '}'
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
}

int  main( int argc, char **argv )
{
	if( argc != 2 ) {
		fprintf(  stdout,  "Usage:  ./parser  [filename]\n"  );
		exit(0);
	}

	FILE *fp = fopen( argv[1], "r" );
	
	if( fp == NULL )  {
		fprintf( stdout, "Open  file  error\n" );
		exit(-1);
	}
	
	yyin = fp;
	yyparse();

	fprintf( stdout, "\n" );
	fprintf( stdout, "|--------------------------------|\n" );
	fprintf( stdout, "|  There is no syntactic error!  |\n" );
	fprintf( stdout, "|--------------------------------|\n" );
	exit(0);
}

