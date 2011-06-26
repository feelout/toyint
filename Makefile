OBJS = lexer.o ast.o parser.o scope.o interpreter.o types.o idtable.o modules.o
TEST_MODULES = tests/foreign_func.so
LIBS = -ldl
CFLAGS = -g -rdynamic

all: toycmp hashtest tokenize

toycmp: main.o $(OBJS)
	gcc $(CFLAGS) -o toycmp main.o $(OBJS) $(LIBS)

hashtest: hashtest.o $(OBJS)
	gcc $(CFLAGS) -o hashtest hashtest.o $(OBJS) $(LIBS)

tokenize: tokenize.o $(OBJS)
	gcc $(CFLAGS) -o tokenize tokenize.o $(OBJS) $(LIBS)

%.o: %.c
	gcc $(CFLAGS) -c $*.c $(LIBS)

%.so: %.c
	gcc -Wall -I. -fPIC -c $*.c -o $*.o
	gcc -shared -Wl,-soname,$*.so -L. -o $*.so $*.o

tests: $(TEST_MODULES)

clean:
	rm -rf toycmp hashtest tokenize *.o tests/*.o $(TEST_MODULES)
