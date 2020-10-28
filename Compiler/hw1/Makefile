all:
	flex -+ scanner.l
	g++ -o scanner -std=c++11 lex.yy.cc -ll

clean:
	rm -f ./scanner ./lex.yy.cc
