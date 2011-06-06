#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "lexer.h"

#define READ_BUF_SIZE	4096
#define EOF_CHAR	'\0'
#define KEYWORD_NUM	11

char* id_table[MAX_ID_NUMBER];
int id_count = 0;
FILE* stream = NULL;
char stream_buffer[READ_BUF_SIZE];
char token_buffer[MAX_TOKEN_SIZE];
int head = 0;
int absolute_head_position = 0;
int buffer_size;

const char *keywords[KEYWORD_NUM] = {"begin", "end", "and", "or", "not", 
	"while", "do", "if", "then", "else", "call"};
int keyword_token_types[KEYWORD_NUM] = {TOKEN_BEGIN, TOKEN_END, TOKEN_AND, 
	TOKEN_OR, TOKEN_NOT, TOKEN_WHILE, TOKEN_DO, TOKEN_IF, 
	TOKEN_THEN, TOKEN_ELSE, TOKEN_CALL};

void fail(const char *message) {
	fprintf(stderr, "ERROR : %s at [%d]\n", message, absolute_head_position);
	exit(-1);
}

void failWithInvalidSymbol(char expected, char got) {
	fprintf(stderr, "ERROR : Expected symbol \'%c\', got \'%c\' at [%d]\n",
			expected, got, absolute_head_position);
	exit(-1);
}

void StartLexer(const char *filename) {
	memset(id_table, 0, sizeof(char*) * MAX_ID_NUMBER);

	stream = fopen(filename, "r");

	if(!stream)
		fail("Failed to open an input stream");
	
	buffer_size = fread(stream_buffer, sizeof(char*), READ_BUF_SIZE, stream);	
}

/* Get next character in the stream.*/
char next_char() {
	return stream_buffer[head];
}

/* Same as next_char, but advances head */
char next_char_and_advance() {
	char c = next_char();

	if(head == buffer_size) {
		if(feof(stream)) {
			return EOF_CHAR;
		} else {
			buffer_size = fread(stream_buffer, sizeof(char*), READ_BUF_SIZE, stream);	
			head = 0;
		}
	}

	++head;
	++absolute_head_position;
	return c;
}

/* Advances stream head */
void advance_head() {
	++head;
}

/* Skip all whitespaces and return the first
 * non-whitespace character */
void SkipWhitespace() {
	char c;

	while((c = next_char()) && isspace(c)) {
		advance_head();
	}
}

void ClearTokenBuffer() {
	memset(token_buffer, sizeof(char*) * MAX_TOKEN_SIZE, '\0');
}

/* Reads alphanumeric characters from stream into token_buffer */
void ReadAlphaNumericTillWS() {
	int token_head = 0;
	char c;

	ClearTokenBuffer();

	do {
		c = next_char();
		if(!isalnum(c)) {
			break;
		}
		token_buffer[token_head++] = c;
		advance_head();
	} while(token_head < MAX_TOKEN_SIZE-1);

	token_buffer[token_head] = '\0';
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

int GetIDIndex(char *s) {
	int n;
	for(n = 0; n < id_count; ++n) {
		if(strcmp(s, id_table[n]) == 0)
			return n;
	}

	char *id_name = (char*)malloc(sizeof(char) * (strlen(s) + 1));
	strcpy(id_name, s);

	id_table[id_count] = id_name;

	return id_count++;
}

Token GetNextToken() {
	Token token;

	SkipWhitespace();
	char c = next_char();
	int nvalue;

	if(isalpha(c)) { /* ID or keyword */
		ReadAlphaNumericTillWS();
		/* TODO: If ID, add to symbol table, return ID token*/

		int kwtype = GetKeywordToken(token_buffer);

		if(kwtype == TOKEN_UNKNOWN) { /* ID */
			nvalue = GetIDIndex(token_buffer);		

			token.type = TOKEN_ID;
			token.value = nvalue;
		} else { /* Keyword */
			token.type = kwtype;
		}
	} else if(isdigit(c)) { /* Constant */
		ReadAlphaNumericTillWS();
		
		nvalue = atoi(token_buffer);
		
		token.type = TOKEN_NUMBER;
		token.value = nvalue;
	} else if(c == ':') {
		advance_head();
		c = next_char_and_advance();

		if(c != '=')
			failWithInvalidSymbol('=', c);

		token.type = TOKEN_ASSIGNMENT;
	} else if(c == ';') {
		advance_head();

		token.type = TOKEN_SEMICOLON;
	} else if(c == '<') {
		advance_head();
		c = next_char_and_advance();

		if(c == '=')
			token.type = TOKEN_LTE;
		else
			token.type = TOKEN_LT;
	} else if(c == '>') {
		advance_head();
		c = next_char_and_advance();

		if(c == '=')
			token.type = TOKEN_GTE;
		else
			token.type = TOKEN_GT;
	} else if(c == '=') {
		advance_head();

		token.type = TOKEN_EQ;
	} else if(c == '+') {
		advance_head();

		token.type = TOKEN_PLUS;
	} else if(c == '-') {
		advance_head();
		
		token.type = TOKEN_MINUS;
	} else if(c == '*') {
		advance_head();

		token.type = TOKEN_STAR;
	} else if(c == '/') {
		advance_head();

		token.type = TOKEN_SLASH;
	} else if(c == '(') {
		advance_head();

		token.type = TOKEN_LEFTBRACKET;
	} else if(c == ')') {
		advance_head();

		token.type = TOKEN_RIGHTBRACKET;
	} else if(c == EOF_CHAR) {
		token.type = TOKEN_EOF;
	} else {
		failWithInvalidSymbol('?', c);
	}

	return token;
}

void StopLexer() {
	fclose(stream);
}
