#include <stdio.h>
#include "lexer.h"
#include "parser.h"

int main(int argc, char *argv[]) {
	//Token t;
	LexerState lex;
	StartLexer(&lex, "tests/hello.toy");
	/*do {
		t = GetNextToken(&lex);
		printf("%d\n", t.type);
	} while(t.type != TOKEN_EOF);*/
	AST *ast = ParseFile("tests/hello.toy");
	DumpAST(ast);
	printf("\n");
	return 0;
}
