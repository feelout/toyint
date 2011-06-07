toycmp: main.c lexer.c ast.c parser.c lexer.h ast.h parser.h scope.h scope.c
	gcc -g -o toycmp -Wall main.c lexer.c ast.c parser.c scope.c

clean:
	rm -rf toycmp *.o
