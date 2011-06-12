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

char symbols[] = {';', '=', '+', '-', '*', '/', '(', ')', '[', ']', ',', EOF_CHAR};
enum TokenType symbol_token_types[] = {TOKEN_SEMICOLON, TOKEN_EQ, TOKEN_PLUS, TOKEN_MINUS, 
	TOKEN_STAR, TOKEN_SLASH, TOKEN_LEFTBRACKET, TOKEN_RIGHTBRACKET, 
	TOKEN_LEFT_SQUARE_BRACKET, TOKEN_RIGHT_SQUARE_BRACKET, TOKEN_COMMA, TOKEN_EOF};

char* token_name[] = {
	"unknown", "begin", "end", "id", ":=", "const",
	";", "and", "or", "not", "<", "=", ">", "<=", "=>",
	"+", "-", "*", "/", "(", ")", "while", "do", "if",
	"then", "else", "EOF", "call", "print", "intread",
	"read", "string", "function", ",", "local", "return",
	"array", "[", "]",
};

/* Fails with given message */
void fail(LexerState *lex, const char *message) {
	fprintf(stderr, "ERROR : %s at [%d]\n", message, lex->absolute_head_position);
	StopLexer(lex);
	exit(-1);
}

/* Fails in case of unexpected symbol (? is used in case of ambiguity) */
void failWithInvalidSymbol(LexerState *lex, char expected, char got) {
	fprintf(stderr, "ERROR : Expected symbol \'%c\', got \'%c\' at [%d]\n",
			expected, got, lex->absolute_head_position);
	StopLexer(lex);
	exit(-1);
}

/* Starts lexer on given source file */
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
char NextChar(LexerState *lex) {
	return lex->stream_buffer[lex->head];
}

/* Same as NextChar, but advances head */
char NextCharAndAdvance(LexerState *lex) {
	char c = NextChar(lex);

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
void AdvanceHead(LexerState *lex) {
	++lex->head;
	++lex->absolute_head_position;
}

/* Skip all whitespaces and return the first
 * non-whitespace character */
void SkipWhitespace(LexerState *lex) {
	char c;

	while((c = NextChar(lex)) && isspace(c)) {
		if(c == '\n')
			++lex->line_num;
		AdvanceHead(lex);
	}
}

/* Empties lexer's token buffer */
void ClearTokenBuffer(LexerState *lex) {
	memset(lex->token_buffer, sizeof(char*) * MAX_TOKEN_SIZE, '\0');
}

/* Reads symbols while they satisfy given predicate. After exection,
 * lexer head points to the first non-satisfying symbol */
void ReadSymbols(LexerState *lex, int (*allowed_tester)(char)) {
	int token_head = 0;
	char c;

	ClearTokenBuffer(lex);

	do {
		c = NextChar(lex);
		if(!allowed_tester(c)) {
			break;
		}
		lex->token_buffer[token_head++] = c;
		AdvanceHead(lex);
	} while(token_head < MAX_TOKEN_SIZE-1);

	lex->token_buffer[token_head] = '\0';
}

/* Checks if char is valid id symbol (though currently
 * all ids must start with an alpha */
int IsIDSymbol(char c) {
	return isalnum(c) || c == '_';
}

/* Reads identifier */
void ReadIDSymbolsTillWS(LexerState *lex) {
	ReadSymbols(lex, IsIDSymbol);
}

/* Predicate, which fails only for string literals' delimiters */
int IsNotStringEnd(char c) {
	/* Allow line-spanning constants for now */
	return c != '\"';
}

/* Reads string literal (head must point to the first non-delimiting symbol */
void ReadStringLiteral(LexerState *lex) {
	ReadSymbols(lex, IsNotStringEnd);
}

/* Returns keyword token type if the string is reserved keyword.
 * Otherwise, returns TOKEN_UNKNOWN */
enum TokenType GetKeywordToken(char *s) {
	int i;	
	for(i = 0; i < KEYWORD_NUM; ++i) {
		if(strcmp(keywords[i], s) == 0) {
			return keyword_token_types[i];
		}
	}

	return TOKEN_UNKNOWN;
}

/* Takes id name and returns it's number in id table, adding
 * it there if necessary */
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

/* Yields next token */
Token GetNextToken(LexerState *lex) {
	Token token;

	SkipWhitespace(lex);

	token.line_num = lex->line_num;

	char c = NextChar(lex);
	int nvalue;
	char *string_literal_buffer;

	if(isalpha(c)) { /* ID or keyword */
		ReadIDSymbolsTillWS(lex);

		int kwtype = GetKeywordToken(lex->token_buffer);

		if(kwtype == TOKEN_UNKNOWN) { /* ID */
			nvalue = GetIDIndex(lex, lex->token_buffer);		

			token.type = TOKEN_ID;
			token.value = CreateIntegralValue(nvalue);
		} else { /* Keyword */
			token.type = kwtype;
		}
	} else if(isdigit(c)) { /* Integral constant */
		ReadIDSymbolsTillWS(lex);
		
		nvalue = atoi(lex->token_buffer);
		
		token.type = TOKEN_NUMBER;
		token.value = CreateIntegralValue(nvalue);
	} else if(c == '\"') {
		AdvanceHead(lex); /* Beginning quotes */

		ReadStringLiteral(lex);

		string_literal_buffer = (char*)malloc(sizeof(char) * (strlen(lex->token_buffer) + 1));
		strcpy(string_literal_buffer, lex->token_buffer);
		token.type = TOKEN_STRING;
		token.value = CreateStringValue(string_literal_buffer);

		AdvanceHead(lex); /* Ending quotes */
	} else if(c == ':') {
		AdvanceHead(lex);
		c = NextCharAndAdvance(lex);

		if(c != '=')
			failWithInvalidSymbol(lex, '=', c);

		token.type = TOKEN_ASSIGNMENT;
	} else if(c == '<') {
		AdvanceHead(lex);
		c = NextCharAndAdvance(lex);

		if(c == '=')
			token.type = TOKEN_LTE;
		else
			token.type = TOKEN_LT;
	} else if(c == '>') {
		AdvanceHead(lex);
		c = NextCharAndAdvance(lex);

		if(c == '=')
			token.type = TOKEN_GTE;
		else
			token.type = TOKEN_GT;
	} else {

		int i, found = 0;
		for(i = 0; i < sizeof(symbols) / sizeof(char); ++i) {
			if(c == symbols[i]) {
				found = 1;
				AdvanceHead(lex);
				token.type = symbol_token_types[i];
				break;
			}
		}

		if(!found) {
			failWithInvalidSymbol(lex, '?', c);
		}
	}

	return token;
}

/* Stops the lexer */
void StopLexer(LexerState *lex) {
	fclose(lex->stream);
}
