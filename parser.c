#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "parser.h"
#include "lexer.h"

typedef struct {
	LexerState* lex;
	Token currentToken;
	Token nextToken;
} ParserState;

void FailWithUnexpectedToken(ParserState* parser, int got, int needed) {
	fprintf(stderr, "Unexpected token at line %d: \"%s\", needed \"%s\"\n", 
				parser->currentToken.line_num, token_name[got], token_name[needed]);
	exit(-1);
}

void FailWithParsingError(char* msg) {
	fprintf(stderr, "Parsing error : %s\n", msg);
	exit(-1);
}

void Advance(ParserState* parser) {
	parser->currentToken = parser->nextToken;
	if(parser->currentToken.type != TOKEN_EOF)
		parser->nextToken = GetNextToken(parser->lex);
}

Value* Match(ParserState* parser, int expectedToken) {
	if(parser->currentToken.type != expectedToken) {
		FailWithUnexpectedToken(parser, parser->currentToken.type, expectedToken);
	}

	Value* value = parser->currentToken.value;

	Advance(parser);

	return value;
}

AST* ParseProgram(ParserState* parser);
AST* ParseBlock(ParserState* parser);
AST* ParseCondition(ParserState* parser);
AST* ParseLogicalExpression(ParserState* parser);
AST* ParseOperatorList(ParserState* parser);
AST* ParseOperator(ParserState* parser);
AST* ParseAssignment(ParserState* parser);
AST* ParseWhileCycle(ParserState* parser);
AST* ParseIfStatement(ParserState* parser);
AST* ParseFunctionCall(ParserState* parser);
AST* ParseArgumentList(ParserState* parser);
AST* ParseExpression(ParserState* parser);
AST* ParseArithmeticalExpression(ParserState* parser);
AST* ParseTerm(ParserState* parser);
AST* ParseValue(ParserState* parser);
AST* ParsePrint(ParserState* parser);
AST* ParseFunctionDefinition(ParserState* parser);
AST* ParseReturn(ParserState* parser);
AST* ParseArray(ParserState* parser);
AST* ParseObject(ParserState* parser);

AST* ParseFile(char* filename, IDTable* id_table) {
	ParserState parser;

	parser.lex = (LexerState*)malloc(sizeof(LexerState));
	StartLexer(parser.lex, filename, id_table);

	AST* program = ParseProgram(&parser);
	
	printf("ID table : \n");
	int i;
	for(i = 0; i < id_table->count; ++i) {
		printf("%d = %s\n", i, id_table->names[i]);
	}

	free(parser.lex);

	return program;
}

AST* ParseProgram(ParserState* parser) {
	parser->nextToken = GetNextToken(parser->lex);
	Advance(parser);
	return ParseBlock(parser);
}

/* Block does not create new node */
AST* ParseBlock(ParserState* parser) {
	Match(parser, TOKEN_BEGIN);
	AST* oplist = ParseOperatorList(parser);
	Match(parser, TOKEN_END);

	return oplist;
}

AST* ParseOperatorList(ParserState* parser) {
	AST *operators = CreateASTNode(SEM_EMPTY, VALUE_EMPTY);

	while(parser->currentToken.type != TOKEN_END) {
		AST *op = ParseOperator(parser);
		AddASTChild(operators, op);
	}

	return operators;
}

AST* ParseOperator(ParserState* parser) {
	AST* ast;
	switch(parser->currentToken.type) {
		case TOKEN_BEGIN:
			return ParseBlock(parser);
		case TOKEN_ID:
			ast = parser->nextToken.type == TOKEN_LEFTBRACKET ? ParseFunctionCall(parser) : ParseAssignment(parser);
			Match(parser, TOKEN_SEMICOLON);
			return ast;
		case TOKEN_LOCAL:
			ast = ParseAssignment(parser);
			Match(parser, TOKEN_SEMICOLON);
			return ast;
		case TOKEN_WHILE:
			return ParseWhileCycle(parser);
		case TOKEN_IF:
			return ParseIfStatement(parser);
		case TOKEN_FUNCTION:
			return ParseFunctionDefinition(parser);
		case TOKEN_RETURN:
			ast = ParseReturn(parser);
			Match(parser, TOKEN_SEMICOLON);
			return ast;
		case TOKEN_PRINT:
			ast =  ParsePrint(parser);
			Match(parser, TOKEN_SEMICOLON);
			return ast;
	}

	FailWithUnexpectedToken(parser, parser->currentToken.type, TOKEN_ID);

	return NULL;
}

AST* ParseArrayIndexing(ParserState* parser) {
	Value* id = Match(parser, TOKEN_ID);

	Match(parser, TOKEN_LEFT_SQUARE_BRACKET);
	AST* index = ParseExpression(parser);
	Match(parser, TOKEN_RIGHT_SQUARE_BRACKET);

	AST* ast = CreateASTNode(SEM_INDEX, id);
	AddASTChild(ast, index);

	return ast;
}

AST* ParseField(ParserState* parser) {
	Value* id = Match(parser, TOKEN_ID);

	Match(parser, TOKEN_DOT);

	Value* field_name = Match(parser, TOKEN_FIELD);

	AST* ast = CreateASTNode(SEM_FIELD, id);
	AddASTChild(ast, CreateASTNode(SEM_CONSTANT, field_name));

	return ast;
}

AST* ParseAssignment(ParserState* parser) {
	AST *lvalue;
	enum Semantic sem = SEM_ASSIGNMENT;

	if(parser->currentToken.type == TOKEN_LOCAL) {
		Match(parser, TOKEN_LOCAL);
		sem = SEM_LOCAL_ASSIGNMENT;
	}

	if(parser->nextToken.type == TOKEN_LEFT_SQUARE_BRACKET) {
		lvalue = ParseArrayIndexing(parser);
	} else if(parser->nextToken.type == TOKEN_DOT) {
		lvalue = ParseField(parser);
	} else {
		Value *id = Match(parser, TOKEN_ID);
		lvalue = CreateASTNode(SEM_ID, id);
	}

	Match(parser, TOKEN_ASSIGNMENT);

	AST *expr = ParseExpression(parser);

	AST *assignment = CreateASTNode(sem, VALUE_EMPTY);

	AddASTChild(assignment, lvalue);
	AddASTChild(assignment, expr);

	return assignment;
}

AST* ParsePrint(ParserState* parser) {
	Match(parser, TOKEN_PRINT);

	AST* printNode = CreateASTNode(SEM_PRINT, VALUE_EMPTY);
	AST* exprNode = ParseExpression(parser);

	printNode->child = exprNode;

	return printNode;
}

AST* ParseFunctionDefinition(ParserState* parser) {
	Match(parser, TOKEN_FUNCTION);
	Value* name = NULL;
	if(parser->currentToken.type == TOKEN_ID) {
		name = Match(parser, TOKEN_ID);
	}

	AST* function = CreateASTNode(SEM_FUNCTION, name);

	Match(parser, TOKEN_LEFTBRACKET);
	AST* arglist = ParseArgumentList(parser);
	Match(parser, TOKEN_RIGHTBRACKET);

	AST* code = ParseBlock(parser);

	AddASTChild(function, arglist);
	AddASTChild(function, code);

	return function;
}

AST* ParseWhileCycle(ParserState* parser) {
	Match(parser, TOKEN_WHILE);

	AST* whileNode = CreateASTNode(SEM_WHILE_CYCLE, VALUE_EMPTY);

	AST* condition = ParseCondition(parser);
	
	Match(parser, TOKEN_DO);

	AST* op = ParseOperator(parser);

	AddASTChild(whileNode, condition);
	AddASTChild(whileNode, op);

	return whileNode;
}

/* Without priorities yet */
AST* ParseCondition(ParserState* parser) {
	if(parser->currentToken.type == TOKEN_NOT) {
		AST* notCond = CreateASTNode(SEM_NOT, VALUE_EMPTY);
		Match(parser, TOKEN_NOT);

		AddASTChild(notCond, ParseLogicalExpression(parser));

		return notCond;
	} else {
		AST* lexp = ParseLogicalExpression(parser);
		AST* rhs;

		enum TokenType type = parser->currentToken.type;

		if(type == TOKEN_AND || type == TOKEN_OR) {
			Match(parser, type);
			rhs = ParseCondition(parser);

			AST *log_node = CreateASTNode(type == TOKEN_AND ? SEM_AND : SEM_OR, VALUE_EMPTY);
			AddASTChild(log_node, lexp);
			AddASTChild(log_node, rhs);

			return log_node;
		}

		return lexp;
	}
}

AST* ParseLogicalExpression(ParserState* parser) {
	AST* valueNode = ParseValue(parser);
	AST* lexpNode = NULL;

	switch(parser->currentToken.type) {
		case TOKEN_LT:
			lexpNode = CreateASTNode(SEM_LT, VALUE_EMPTY);
			break;
		case TOKEN_EQ:
			lexpNode = CreateASTNode(SEM_EQ, VALUE_EMPTY);
			break;
		case TOKEN_GT:
			lexpNode = CreateASTNode(SEM_GT, VALUE_EMPTY);
			break;
		case TOKEN_LTE:
			lexpNode = CreateASTNode(SEM_LTE, VALUE_EMPTY);
			break;
		case TOKEN_GTE:
			lexpNode = CreateASTNode(SEM_GTE, VALUE_EMPTY);
			break;
		default:
			return valueNode;
	}

	Advance(parser);

	AST* rhs = ParseValue(parser);

	AddASTChild(lexpNode, valueNode);
	AddASTChild(lexpNode, rhs);

	return lexpNode;
}

AST* ParseIfStatement(ParserState* parser) {
	Match(parser, TOKEN_IF);

	AST* ifNode = CreateASTNode(SEM_IF_STATEMENT, VALUE_EMPTY);

	AST* condition = ParseCondition(parser);

	Match(parser, TOKEN_THEN);

	AST* thenOp = ParseOperator(parser);

	AddASTChild(ifNode, condition);
	AddASTChild(ifNode, thenOp);

	if(parser->currentToken.type == TOKEN_ELSE) {
		Match(parser, TOKEN_ELSE);
		AST* elseOp = ParseOperator(parser);
		AddASTChild(ifNode, elseOp);
	}

	return ifNode;
}

AST* ParseFunctionCall(ParserState* parser) {
	Value* funcId = Match(parser, TOKEN_ID);

	AST* funcNode = CreateASTNode(SEM_FUNCCALL, funcId);

	Match(parser, TOKEN_LEFTBRACKET);
	AST* arglist = ParseArgumentList(parser);
	Match(parser, TOKEN_RIGHTBRACKET);

	AddASTChild(funcNode, arglist);

	return funcNode;
}

AST* ParseArgumentList(ParserState* parser) {
	AST* arglist = CreateASTNode(SEM_EMPTY, VALUE_EMPTY);	

	/* FIXME: At this time, ArgumentList is used both for calling
	 * and defining functions, which is wrong as it allows using
	 * constants as formal arguments of a function */
	while(parser->currentToken.type != TOKEN_RIGHTBRACKET) {
		AST* argument = ParseValue(parser);
		AddASTChild(arglist, argument);
		if(parser->currentToken.type != TOKEN_RIGHTBRACKET)
			Match(parser, TOKEN_COMMA);
	}

	return arglist;
}

AST* ParseExpression(ParserState* parser) {
	switch(parser->currentToken.type) {
		case TOKEN_INTREAD:
			Match(parser, TOKEN_INTREAD);
			return CreateASTNode(SEM_INTREAD, VALUE_EMPTY);
		case TOKEN_READ:
			Match(parser, TOKEN_READ);
			return CreateASTNode(SEM_READ, VALUE_EMPTY);
		case TOKEN_ARRAY:
			return ParseArray(parser);
		case TOKEN_OBJECT:
			return ParseObject(parser);
		case TOKEN_FUNCTION:
			return ParseFunctionDefinition(parser);
		default:
			return ParseArithmeticalExpression(parser);
	}

	assert(0);
	return NULL;
}

AST* ParseArithmeticalExpression(ParserState* parser) {
	AST* termNode = ParseTerm(parser);

	if(parser->currentToken.type == TOKEN_PLUS) {
		Match(parser, TOKEN_PLUS);

		AST* exprNode = CreateASTNode(SEM_ADDITION, VALUE_EMPTY);
		AddASTChild(exprNode, termNode);
		AddASTChild(exprNode, ParseArithmeticalExpression(parser));

		return exprNode;
	} else if(parser->currentToken.type == TOKEN_MINUS) {
		Match(parser, TOKEN_MINUS);

		AST* exprNode = CreateASTNode(SEM_SUBTRACTION, VALUE_EMPTY);
		AddASTChild(exprNode, termNode);
		AddASTChild(exprNode, ParseArithmeticalExpression(parser));

		return exprNode;
	} else {
		return termNode;
	}
}

AST* ParseTerm(ParserState* parser) {
	AST* valueNode = ParseValue(parser);

	if(parser->currentToken.type == TOKEN_STAR) {
		Match(parser, TOKEN_STAR);

		AST* exprNode = CreateASTNode(SEM_MULTIPLICATION, VALUE_EMPTY);
		AddASTChild(exprNode, valueNode);
		AddASTChild(exprNode, ParseTerm(parser));

		return exprNode;
	} else if(parser->currentToken.type == TOKEN_SLASH) {
		Match(parser, TOKEN_SLASH);

		AST* exprNode = CreateASTNode(SEM_DIVISION, VALUE_EMPTY);
		AddASTChild(exprNode, valueNode);
		AddASTChild(exprNode, ParseTerm(parser));

		return exprNode;
	} else {
		return valueNode;
	}
}

AST* ParseValue(ParserState* parser) {
	AST* node;
	if(parser->currentToken.type == TOKEN_NUMBER || parser->currentToken.type == TOKEN_STRING) {
		node =  CreateASTNode(SEM_CONSTANT, parser->currentToken.value);
		Match(parser, parser->currentToken.type);
	} else {
		if(parser->nextToken.type == TOKEN_LEFTBRACKET)
			return ParseFunctionCall(parser);

		Value *id = Match(parser, TOKEN_ID);

		/* XXX: Factor this out */
		if(parser->currentToken.type == TOKEN_LEFT_SQUARE_BRACKET) {
			Match(parser, TOKEN_LEFT_SQUARE_BRACKET);
			AST* size = ParseExpression(parser);
			Match(parser, TOKEN_RIGHT_SQUARE_BRACKET);

			node = CreateASTNode(SEM_INDEX,  id);
			AddASTChild(node, size);
		} else if(parser->currentToken.type == TOKEN_DOT) { 
			Match(parser, TOKEN_DOT);
			Value* field_name = Match(parser, TOKEN_FIELD);
			if(parser->currentToken.type == TOKEN_LEFTBRACKET) {
				node = CreateASTNode(SEM_METHOD_CALL, VALUE_EMPTY);

				AST* field_node = CreateASTNode(SEM_FIELD, id);
				AddASTChild(field_node, CreateASTNode(SEM_CONSTANT, field_name));

				AddASTChild(node, field_node);

				Match(parser, TOKEN_LEFTBRACKET);
				AddASTChild(node, ParseArgumentList(parser)); Match(parser, TOKEN_RIGHTBRACKET); } else {
				node = CreateASTNode(SEM_FIELD, id);
				AddASTChild(node, CreateASTNode(SEM_CONSTANT, field_name));
			}
		} else {
			node =  CreateASTNode(SEM_ID, id);
		}
	}

	return node;
}

AST* ParseReturn(ParserState* parser) {
	Match(parser, TOKEN_RETURN);

	AST* node = CreateASTNode(SEM_RETURN, VALUE_EMPTY);
	AST* ret_expr = ParseExpression(parser);

	AddASTChild(node, ret_expr);

	return node;
}

AST* ParseArray(ParserState* parser) {
	Match(parser, TOKEN_ARRAY);
	Match(parser, TOKEN_LEFT_SQUARE_BRACKET);

	AST* size_expr = ParseExpression(parser);

	Match(parser, TOKEN_RIGHT_SQUARE_BRACKET);

	AST* node = CreateASTNode(SEM_ARRAY, VALUE_EMPTY);
	AddASTChild(node, size_expr);

	return node;
}

AST* ParseObject(ParserState* parser) {
	Match(parser, TOKEN_OBJECT);

	return CreateASTNode(SEM_OBJECT, VALUE_EMPTY);
}
