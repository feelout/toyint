OBJS = lexer.o ast.o parser.o scope.o interpreter.o types.o

all: toycmp hashtest

toycmp: main.o $(OBJS)
	gcc -g -o toycmp main.o $(OBJS)

hashtest: hashtest.o $(OBJS)
	gcc -g -o hashtest hashtest.o $(OBJS)

%.o: %.c
	gcc -g -c $*.c

clean:
	rm -rf toycmp hashtest *.o
