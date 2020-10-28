TARGET = parser
OBJECT = lex.yy.c y.tab.c y.tab.o lex.yy.o y.output parser y.tab.h main.o sym_table.o
CXX = gcc -g
CXX = g++ -g -I./
LEX = flex
YACXX = yacc -v
YACXXFLAG = -d
LIBS = -lfl

all: $(TARGET)

parser: y.tab.o lex.yy.o main.o sym_table.o
	$(CXX) -o $(TARGET) $^ $(LIBS)

y.tab.o: y.tab.c
	$(CXX) -c y.tab.c

y.tab.c: parser.y
	$(YACXX) $(YACXXFLAG) parser.y

lex.yy.o: lex.yy.c
	$(CC) -c lex.yy.c

lex.yy.c: lex.l
	$(LEX) lex.l

main.o: main.c
	$(CXX) -c main.c

sym_table.o: symbol_table.cpp
	$(CXX) -c $^ -o $@

clean:
	rm -f $(TARGET) $(OBJECT)

