#makefile for scanner
LEX = lex
YACC = bison
LEXFN = lex.l
CC = gcc
CXX = g++
CFLAGS = -O2
LIBS = -ll -ly
RM = rm -f
TESTFN = test
all: parser

parser: yacc lex
	$(CC) -o $@ lex.yy.c yacc.tab.c $(LIBS)
lex:
	$(LEX) -o lex.yy.c $(LEXFN) 
yacc:
	$(YACC) -d -v yacc.y
test:
	./yacc $(TESTFN) 
clean:
	$(RM) ./lex.yy.c ./yacc.tab.h ./yacc.tab.c ./parser ./yacc.output
	

