#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "lexer.h"

#define EOF_CHAR	'\0'
#define KEYWORD_NUM	18

const char *keywords[KEYWORD_NUM] = {"begin", "end", "and", "or", "not", 
	"while", "do", "if", "then", "else", "call", "print", "intread", 
	"read", "function", "local", "return", "array"};
int keyword_token_types[KEYWORD_NUM] = {TOKEN_BEGIN, TOKEN_END, TOKEN_AND, 
	TOKEN_OR, TOKEN_NOT, TOKEN_WHILE, TOKEN_DO, TOKEN_IF,
	TOKEN_THEN, TOKEN_ELSE, TOKEN_CALL, TOKEN_PRINT, TOKEN_INTREAD, 
	TOKEN_READ, TOKEN_FUNCTION, TOKEN_LOCAL, TOKEN_RETURN, TOKEN_ARRAY};

char* tokenName[] = {
	"unknown", "begin", "end", "id", ":=", "const",
	";", "and", "or", "not", "<", "=", ">", "<=", "=>",
	"+", "-", "*", "/", "(", ")", "while", "do", "if",
	"then", "else", "EOF", "call", "print", "intread",
	"read", "string", "function", ",", "local", "return",
	"array", "[", "]",
};

void fail(LexerState *lex, const char *message) {
	fprintf(stderr, "ERROR : %s at [%d]\n", message, lex->absolute_head_position);
	StopLexer(lex);
	exit(-1);
}

void failWithInvalidSymbol(LexerState *lex, char expected, char got) {
	fprintf(stderr, "ERROR : Expected symbol \'%c\', got \'%c\' at [%d]\n",
			expected, got, lex->absolute_head_position);
	StopLexer(lex);
	exit(-1);
}

void StartLexer(LexerState *lex, const char *filename) {
	lex->stream = NULL;
	lex->head = 0;
	lex->absolute_head_position = 0;

	memset(lex->id_names, 0, sizeof(char*) * MAX_ID_NUMBER);
	memset(lex->stream_buffer, 0, sizeof(char) * READ_BUF_SIZE);
	memset(lex->token_buffer, 0, sizeof(char) * MAX_TOKEN_SIZE);

	lex->id_names[RETURN_VALUE_ID] = "__retvalue__";
	lex->id_count = 0;

	lex->stream = fopen(filename, "r");

	if(!lex->stream)
		fail(lex, "Failed to open an input stream");
	
	lex->buffer_size = fread(lex->stream_buffer, sizeof(char), READ_BUF_SIZE, lex->stream);	
	lex->line_num = 0;
}

/* Get next character in the stream.*/
char next_char(LexerState *lex) {
	return lex->stream_buffer[lex->head];
}

/* Same as next_char, but advances head */
char next_char_and_advance(LexerState *lex) {
	char c = next_char(lex);

	if(lex->head == lex->buffer_size) {
		if(feof(lex->stream)) {
			return EOF_CHAR;
		} else {
			lex->buffer_size = fread(lex->stream_buffer, sizeof(char*), READ_BUF_SIZE, lex->stream);	
			lex->head = 0;
		}
	}

	++lex->head;
	++lex->absolute_head_position;
	return c;
}

/* Advances stream head */
void advance_head(LexerState *lex) {
	++lex->head;
	++lex->absolute_head_position;
}

/* Skip all whitespaces and return the first
 * non-whitespace character */
void SkipWhitespace(LexerState *lex) {
	char c;

	while((c = next_char(lex)) && isspace(c)) {
		if(c == '\n')
			++lex->line_num;
		advance_head(lex);
	}
}

void ClearTokenBuffer(LexerState *lex) {
	memset(lex->token_buffer, sizeof(char*) * MAX_TOKEN_SIZE, '\0');
}

/* Reads alphanumeric characters from stream into token_buffer */
void ReadAlphaNumericTillWS(LexerState *lex) {
	int token_head = 0;
	char c;

	ClearTokenBuffer(lex);

	do {
		c = next_char(lex);
		if(!isalnum(c)) {
			break;
		}
		lex->token_buffer[token_head++] = c;
		advance_head(lex);
	} while(token_head < MAX_TOKEN_SIZE-1);

	lex->token_buffer[token_head] = '\0';
}

// TODO: Unify with previous ASAP
void ReadStringLiteral(LexerState *lex) {
	int token_head = 0;
	char c;

	ClearTokenBuffer(lex);

	c = next_char_and_advance(lex);

	if(c != '\"')
		failWithInvalidSymbol(lex, '\"', c);

	do {
		c = next_char(lex);
		if(c == '\"') {
			advance_head(lex);
			break;
		}
		if(c == '\n')
			fail(lex, "string literals can't span multiple lines");
		lex->token_buffer[token_head++] = c;
		advance_head(lex);
	} while(token_head < MAX_TOKEN_SIZE-1);

	lex->token_buffer[token_head] = '\0';
}

int GetKeywordToken(char *s) {
	int i;	
	for(i = 0; i < KEYWORD_NUM; ++i) {
		if(strcmp(keywords[i], s) == 0) {
			return keyword_token_types[i];
		}
	}

	return TOKEN_UNKNOWN;
}

int GetIDIndex(LexerState *lex, char *s) {
	int n;
	for(n = 0; n < lex->id_count; ++n) {
		if(strcmp(s, lex->id_names[n]) == 0)
			return n;
	}

	char *id_name = (char*)malloc(sizeof(char) * (strlen(s) + 1));
	strcpy(id_name, s);

	lex->id_names[lex->id_count] = id_name;

	return lex->id_count++;
}

/* XXX: REFACTOR THIS ASAP!!! */
Token GetNextToken(LexerState *lex) {
	Token token;

	SkipWhitespace(lex);

	token.line_num = lex->line_num;

	char c = next_char(lex);
	int nvalue;
	char *string_literal_buffer;

	if(isalpha(c)) { /* ID or keyword */
		ReadAlphaNumericTillWS(lex);
		/* TODO: If ID, add to symbol table, return ID token*/

		int kwtype = GetKeywordToken(lex->token_buffer);

		if(kwtype == TOKEN_UNKNOWN) { /* ID */
			nvalue = GetIDIndex(lex, lex->token_buffer);		

			token.type = TOKEN_ID;
			token.value = CreateIntegralValue(nvalue);
		} else { /* Keyword */
			token.type = kwtype;
		}
	} else if(isdigit(c)) { /* Integral constant */
		ReadAlphaNumericTillWS(lex);
		
		nvalue = atoi(lex->token_buffer);
		
		token.type = TOKEN_NUMBER;
		token.value = CreateIntegralValue(nvalue);
	} else if(c == '\"') {
		ReadStringLiteral(lex);

		string_literal_buffer = (char*)malloc(sizeof(char) * (strlen(lex->token_buffer) + 1));
		strcpy(string_literal_buffer, lex->token_buffer);
		token.type = TOKEN_STRING;
		token.value = CreateStringValue(string_literal_buffer);
	} else if(c == ':') {
		advance_head(lex);
		c = next_char_and_advance(lex);

		if(c != '=')
			failWithInvalidSymbol(lex, '=', c);

		token.type = TOKEN_ASSIGNMENT;
	} else if(c == ';') {
		advance_head(lex);

		token.type = TOKEN_SEMICOLON;
	} else if(c == '<') {
		advance_head(lex);
		c = next_char_and_advance(lex);

		if(c == '=')
			token.type = TOKEN_LTE;
		else
			token.type = TOKEN_LT;
	} else if(c == '>') {
		advance_head(lex);
		c = next_char_and_advance(lex);

		if(c == '=')
			token.type = TOKEN_GTE;
		else
			token.type = TOKEN_GT;
	} else if(c == '=') {
		advance_head(lex);

		token.type = TOKEN_EQ;
	} else if(c == '+') {
		advance_head(lex);

		token.type = TOKEN_PLUS;
	} else if(c == '-') {
		advance_head(lex);
		
		token.type = TOKEN_MINUS;
	} else if(c == '*') {
		advance_head(lex);

		token.type = TOKEN_STAR;
	} else if(c == '/') {
		advance_head(lex);

		token.type = TOKEN_SLASH;
	} else if(c == '(') {
		advance_head(lex);

		token.type = TOKEN_LEFTBRACKET;
	} else if(c == ')') {
		advance_head(lex);

		token.type = TOKEN_RIGHTBRACKET;
	} else if(c == '[') {
		advance_head(lex);

		token.type = TOKEN_LEFT_SQUARE_BRACKET;
	} else if(c == ']') {
		advance_head(lex);

		token.type = TOKEN_RIGHT_SQUARE_BRACKET;
	} else if(c == ',') {
		advance_head(lex);

		token.type = TOKEN_COMMA;
	} else if(c == EOF_CHAR) {
		token.type = TOKEN_EOF;
	} else {
		failWithInvalidSymbol(lex, '?', c);
	}

	return token;
}

void StopLexer(LexerState *lex) {
	fclose(lex->stream);
}
