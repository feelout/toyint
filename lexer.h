#ifndef LEXER_H
#define LEXER_H

enum TokenType {
 	TOKEN_UNKNOWN = 0, TOKEN_BEGIN, TOKEN_END, TOKEN_ID,		/* 0 - 3 */
	TOKEN_ASSIGNMENT, TOKEN_NUMBER, TOKEN_SEMICOLON,			/* 4 - 6 */
	TOKEN_AND, TOKEN_OR, TOKEN_NOT, TOKEN_LT, TOKEN_EQ,			/* 7 - 11 */
	TOKEN_GT, TOKEN_LTE, TOKEN_GTE,	TOKEN_PLUS,					/* 12 - 14 */
	TOKEN_MINUS, TOKEN_STAR, TOKEN_SLASH, TOKEN_LEFTBRACKET,	/* 15 - 18 */
	TOKEN_RIGHTBRACKET, TOKEN_WHILE, TOKEN_DO, TOKEN_IF,		/* 19 - 22 */
	TOKEN_THEN, TOKEN_ELSE,	TOKEN_EOF, TOKEN_CALL				/* 23 - 27 */
};

char* tokenName[];

#define MAX_ID_NUMBER		255
#define MAX_TOKEN_SIZE		255
#define READ_BUF_SIZE	4096

typedef struct {
	int	type;
	int	value; /* For constants - value, for ids - index in id table */
} Token;

typedef struct {
	char* id_table[MAX_ID_NUMBER];
	int id_count;
	FILE* stream;
	char stream_buffer[READ_BUF_SIZE];
	char token_buffer[MAX_TOKEN_SIZE];
	int head;
	int absolute_head_position;
	int buffer_size;
} LexerState;

void StartLexer(LexerState *lex, const char *filename);
void StopLexer(LexerState *lex);
Token GetNextToken(LexerState *lex);
int HasNextToken(LexerState *lex);

#endif /* LEXER_H */
