#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parser.h"
#include "interpreter.h"
#include "idtable.h"
#include "modules.h"

/* NB: Unportable */
char* ExtractFolder(const char* path) {
	const char* last_slash = strrchr(path, '/');

	char* result = (char*)malloc(sizeof(char) * (strlen(path) + 1));

	const char* p = path;
	char* q = result;

	while(p != last_slash) {
		*q++ = *p++;
	}

	*q++ = '/';
	*q = '\0';

	return result;
}

int main(int argc, char *argv[]) {
	if(argc < 2) {
		fprintf(stderr, "Error: input file missing\n");
		return -1;
	}

	char* module_folder = ExtractFolder(argv[1]);
	printf("Module folder of %s: %s\n", argv[1], module_folder);

	AddModulePath(module_folder);
	AddModulePath("./");

	IDTable *id_table = CreateIDTable();

	AST *ast = ParseFile(argv[1], id_table);
	DumpAST(ast);

	Scope *scope = CreateScope();
	InterpretAST(ast, scope, id_table);

	printf("\n");
	return 0;
}
