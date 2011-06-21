#include <stdio.h>
#include "lexer.h"
#include "idtable.h"

int main(int argc, char *argv[]) {
	if(argc < 2) {
		fprintf(stderr, "Error: input file missing\n");
		return -1;
	}

	IDTable *id_table = CreateIDTable();

	Token t;
	LexerState lex;
	StartLexer(&lex, argv[1], id_table);
	do {
		t = GetNextToken(&lex);
		printf("%s\n", token_name[t.type]);
	} while(t.type != TOKEN_EOF);

	printf("\n");
	return 0;
}
