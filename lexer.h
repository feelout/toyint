#ifndef LEXER_H
#define LEXER_H

enum TokenType {
 	TOKEN_UNKNOWN = 0, TOKEN_BEGIN, TOKEN_END, TOKEN_ID,		
	TOKEN_ASSIGNMENT, TOKEN_NUMBER, TOKEN_SEMICOLON,
	TOKEN_AND, TOKEN_OR, TOKEN_NOT, TOKEN_LT, TOKEN_EQ,
	TOKEN_GT, TOKEN_LTE, TOKEN_GTE,	TOKEN_PLUS,
	TOKEN_MINUS, TOKEN_STAR, TOKEN_SLASH, TOKEN_LEFTBRACKET,
	TOKEN_RIGHTBRACKET, TOKEN_WHILE, TOKEN_DO, TOKEN_IF,		
	TOKEN_THEN, TOKEN_ELSE,	TOKEN_EOF, TOKEN_CALL		
};

#define MAX_ID_NUMBER		255
#define MAX_TOKEN_SIZE		255

typedef struct {
	int	type;
	int	value; /* For constants - value, for ids - index in id table */
} Token;

extern char* id_table[];

void StartLexer(const char *filename);
void StopLexer();
Token GetNextToken();
int HasNextToken();

#endif /* LEXER_H */
