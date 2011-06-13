#include <stdio.h>
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"

int main(int argc, char *argv[]) {
	/*if(argc < 2) {
		fprintf(stderr, "Error: input file missing\n");
		return -1;
	}*/
	/*Token t;
	LexerState lex;
	StartLexer(&lex, argv[1]);
	do {
		t = GetNextToken(&lex);
		printf("%s\n", token_name[t.type]);
	} while(t.type != TOKEN_EOF);*/

	AST *ast = ParseFile(argv[1]);
	DumpAST(ast);

	Scope *scope = CreateScope();
	InterpretAST(ast, scope);


	printf("\n");
	return 0;
}
