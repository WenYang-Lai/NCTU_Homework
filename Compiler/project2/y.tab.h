/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_Y_TAB_H_INCLUDED
# define YY_YY_Y_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    ID = 258,
    INT_CONST = 259,
    FLOAT_CONST = 260,
    SCIENTIFIC = 261,
    STR_CONST = 262,
    LE_OP = 263,
    NE_OP = 264,
    GE_OP = 265,
    EQ_OP = 266,
    AND_OP = 267,
    OR_OP = 268,
    READ = 269,
    BOOLEAN = 270,
    WHILE = 271,
    DO = 272,
    IF = 273,
    ELSE = 274,
    TRUE = 275,
    FALSE = 276,
    FOR = 277,
    INT = 278,
    PRINT = 279,
    BOOL = 280,
    VOID = 281,
    FLOAT = 282,
    DOUBLE = 283,
    STRING = 284,
    CONTINUE = 285,
    BREAK = 286,
    RETURN = 287,
    CONST = 288,
    L_PAREN = 289,
    R_PAREN = 290,
    COMMA = 291,
    SEMICOLON = 292,
    ML_BRACE = 293,
    MR_BRACE = 294,
    L_BRACE = 295,
    R_BRACE = 296,
    ADD_OP = 297,
    SUB_OP = 298,
    MUL_OP = 299,
    DIV_OP = 300,
    MOD_OP = 301,
    ASSIGN_OP = 302,
    LT_OP = 303,
    GT_OP = 304,
    NOT_OP = 305
  };
#endif
/* Tokens.  */
#define ID 258
#define INT_CONST 259
#define FLOAT_CONST 260
#define SCIENTIFIC 261
#define STR_CONST 262
#define LE_OP 263
#define NE_OP 264
#define GE_OP 265
#define EQ_OP 266
#define AND_OP 267
#define OR_OP 268
#define READ 269
#define BOOLEAN 270
#define WHILE 271
#define DO 272
#define IF 273
#define ELSE 274
#define TRUE 275
#define FALSE 276
#define FOR 277
#define INT 278
#define PRINT 279
#define BOOL 280
#define VOID 281
#define FLOAT 282
#define DOUBLE 283
#define STRING 284
#define CONTINUE 285
#define BREAK 286
#define RETURN 287
#define CONST 288
#define L_PAREN 289
#define R_PAREN 290
#define COMMA 291
#define SEMICOLON 292
#define ML_BRACE 293
#define MR_BRACE 294
#define L_BRACE 295
#define R_BRACE 296
#define ADD_OP 297
#define SUB_OP 298
#define MUL_OP 299
#define DIV_OP 300
#define MOD_OP 301
#define ASSIGN_OP 302
#define LT_OP 303
#define GT_OP 304
#define NOT_OP 305

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_Y_TAB_H_INCLUDED  */
