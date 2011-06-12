OBJS = main.o lexer.o ast.o parser.o scope.o interpreter.o types.o

toycmp: $(OBJS)
	gcc -o toycmp $(OBJS)

%.o: $*.c
	gcc -o $*.o $*.c

clean:
	rm -rf toycmp *.o
