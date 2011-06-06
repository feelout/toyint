#include <stdio.h>
#include "lexer.h"
#include "parser.h"

int main(int argc, char *argv[]) {
	/*Token t;
	StartLexer("tests/hello.toy");
	do {
		t = GetNextToken();
		printf("%d\n", t.type);
	} while(t.type != TOKEN_EOF);*/
	AST *ast = ParseFile("tests/hello.toy");
	printf("\n");
	return 0;
}
