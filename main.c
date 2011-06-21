#include <stdio.h>
#include "parser.h"
#include "interpreter.h"
#include "idtable.h"

int main(int argc, char *argv[]) {
	if(argc < 2) {
		fprintf(stderr, "Error: input file missing\n");
		return -1;
	}

	IDTable *id_table = CreateIDTable();

	AST *ast = ParseFile(argv[1], id_table);
	DumpAST(ast);

	Scope *scope = CreateScope();
	InterpretAST(ast, scope);

	printf("\n");
	return 0;
}
