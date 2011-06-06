toycmp: main.c lexer.c ast.c parser.c lexer.h ast.h parser.h
	gcc -g -o toycmp -Wall main.c lexer.c ast.c parser.c

clean:
	rm -rf toycmp *.o
