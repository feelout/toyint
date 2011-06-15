#include <stdio.h>
#include "parser.h"
#include "interpreter.h"

int main(int argc, char *argv[]) {
	if(argc < 2) {
		fprintf(stderr, "Error: input file missing\n");
		return -1;
	}

	AST *ast = ParseFile(argv[1]);
	DumpAST(ast);

	Scope *scope = CreateScope();
	InterpretAST(ast, scope);

	printf("\n");
	return 0;
}
