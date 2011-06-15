#include <stdio.h>
#include "lexer.h"

int main(int argc, char *argv[]) {
	if(argc < 2) {
		fprintf(stderr, "Error: input file missing\n");
		return -1;
	}

	Token t;
	LexerState lex;
	StartLexer(&lex, argv[1]);
	do {
		t = GetNextToken(&lex);
		printf("%s\n", token_name[t.type]);
	} while(t.type != TOKEN_EOF);

	printf("\n");
	return 0;
}
