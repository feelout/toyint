toycmp: main.c lexer.c ast.c parser.c lexer.h ast.h parser.h scope.h scope.c interpreter.h interpreter.c types.h types.c
	gcc -g -o toycmp -Wall main.c lexer.c ast.c parser.c scope.c interpreter.c types.c

clean:
	rm -rf toycmp *.o
