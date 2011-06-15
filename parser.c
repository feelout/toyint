#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "parser.h"
#include "lexer.h"
#include "ast.h"

LexerState lex;
Token currentToken;
Token nextToken;

void FailWithUnexpectedToken(int got, int needed) {
	fprintf(stderr, "Unexpected token at line %d: \"%s\", needed \"%s\"\n", 
				currentToken.line_num, token_name[got], token_name[needed]);
	exit(-1);
}

void FailWithParsingError(char* msg) {
	fprintf(stderr, "Parsing error : %s\n", msg);
	exit(-1);
}

void Advance() {
	currentToken = nextToken;
	if(currentToken.type != TOKEN_EOF)
		nextToken = GetNextToken(&lex);
}

Value* Match(int expectedToken) {
	if(currentToken.type != expectedToken) {
		FailWithUnexpectedToken(currentToken.type, expectedToken);
	}

	Value* value = currentToken.value;

	Advance();

	return value;
}

AST* ParseProgram();
AST* ParseBlock();
AST* ParseCondition();
AST* ParseLogicalExpression();
AST* ParseOperatorList();
AST* ParseOperator();
AST* ParseAssignment();
AST* ParseWhileCycle();
AST* ParseIfStatement();
AST* ParseFunctionCall();
AST* ParseArgumentList();
AST* ParseExpression();
AST* ParseArithmeticalExpression();
AST* ParseTerm();
AST* ParseValue();
AST* ParsePrint();
AST* ParseFunctionDefinition();
AST* ParseReturn();
AST* ParseArray();

AST* ParseFile(char* filename) {
	StartLexer(&lex, filename);
	return ParseProgram();
}

AST* ParseProgram() {
	nextToken = GetNextToken(&lex);
	Advance();
	return ParseBlock();
}

/* Block does not create new node */
AST* ParseBlock() {
	Match(TOKEN_BEGIN);
	AST* oplist = ParseOperatorList();
	Match(TOKEN_END);

	return oplist;
}

AST* ParseOperatorList() {
	AST *operators = CreateASTNode(SEM_EMPTY, VALUE_EMPTY);

	while(currentToken.type != TOKEN_END) {
		AST *op = ParseOperator();
		AddASTChild(operators, op);
	}

	return operators;
}

AST* ParseOperator() {
	AST* ast;
	switch(currentToken.type) {
		case TOKEN_BEGIN:
			return ParseBlock();
		case TOKEN_ID:
			ast = nextToken.type == TOKEN_LEFTBRACKET ? ParseFunctionCall() : ParseAssignment();
			Match(TOKEN_SEMICOLON);
			return ast;
		case TOKEN_LOCAL:
			ast = ParseAssignment();
			Match(TOKEN_SEMICOLON);
			return ast;
		case TOKEN_WHILE:
			return ParseWhileCycle();
		case TOKEN_IF:
			return ParseIfStatement();
		case TOKEN_FUNCTION:
			return ParseFunctionDefinition();
		case TOKEN_RETURN:
			ast = ParseReturn();
			Match(TOKEN_SEMICOLON);
			return ast;
		case TOKEN_PRINT:
			ast =  ParsePrint();
			Match(TOKEN_SEMICOLON);
			return ast;
	}

	FailWithUnexpectedToken(currentToken.type, TOKEN_ID);

	return NULL;
}

AST* ParseAssignment() {
	Value* id;
	int local = 0, array = 0, field = 0;
	AST *indexExpr;
	Value* field_name;

	if(currentToken.type == TOKEN_LOCAL) {
		local = 1;
		Match(TOKEN_LOCAL);
	}

	id = Match(TOKEN_ID);	
	if(currentToken.type == TOKEN_LEFT_SQUARE_BRACKET) {
		array = 1;
		Match(TOKEN_LEFT_SQUARE_BRACKET);
		indexExpr = ParseExpression();
		Match(TOKEN_RIGHT_SQUARE_BRACKET);
	} else if(currentToken.type == TOKEN_DOT) {
		field = 1;
		Match(TOKEN_DOT);
		field_name = Match(TOKEN_FIELD);
	}

	Match(TOKEN_ASSIGNMENT);

	AST *lvalueNode;

	if(array) {
		lvalueNode = CreateASTNode(SEM_INDEX, id);
		AddASTChild(lvalueNode, indexExpr);
	} else if(field) {
		lvalueNode = CreateASTNode(SEM_FIELD, id);
		AddASTChild(lvalueNode, CreateASTNode(SEM_CONSTANT, field_name));
	} else
		lvalueNode = CreateASTNode(SEM_ID, id);
	AST *expr = ParseExpression();

	AST *assignment = CreateASTNode(local ? SEM_LOCAL_ASSIGNMENT : SEM_ASSIGNMENT, VALUE_EMPTY);

	AddASTChild(assignment, lvalueNode);
	AddASTChild(assignment, expr);

	return assignment;
}

AST* ParsePrint() {
	Match(TOKEN_PRINT);

	AST* printNode = CreateASTNode(SEM_PRINT, VALUE_EMPTY);
	AST* exprNode = ParseExpression();

	printNode->child = exprNode;

	return printNode;
}

AST* ParseFunctionDefinition() {
	Match(TOKEN_FUNCTION);
	Value* name = Match(TOKEN_ID);
	AST* function = CreateASTNode(SEM_FUNCTION, name);

	Match(TOKEN_LEFTBRACKET);
	AST* arglist = ParseArgumentList();
	Match(TOKEN_RIGHTBRACKET);

	AST* code = ParseBlock();

	AddASTChild(function, arglist);
	AddASTChild(function, code);

	return function;
}

AST* ParseWhileCycle() {
	Match(TOKEN_WHILE);

	AST* whileNode = CreateASTNode(SEM_WHILE_CYCLE, VALUE_EMPTY);

	AST* condition = ParseCondition();
	
	Match(TOKEN_DO);

	AST* op = ParseOperator();

	AddASTChild(whileNode, condition);
	AddASTChild(whileNode, op);

	return whileNode;
}

/* Without priorities yet */
AST* ParseCondition() {
	if(currentToken.type == TOKEN_NOT) {
		AST* notCond = CreateASTNode(SEM_NOT, VALUE_EMPTY);
		Match(TOKEN_NOT);

		AddASTChild(notCond, ParseLogicalExpression());

		return notCond;
	} else {
		AST* lexp = ParseLogicalExpression();
		AST* rhs;

		enum TokenType type = currentToken.type;

		if(type == TOKEN_AND || type == TOKEN_OR) {
			Match(type);
			rhs = ParseCondition();

			AST *log_node = CreateASTNode(type == TOKEN_AND ? SEM_AND : SEM_OR, VALUE_EMPTY);
			AddASTChild(log_node, lexp);
			AddASTChild(log_node, rhs);

			return log_node;
		}

		return lexp;
	}
}

AST* ParseLogicalExpression() {
	AST* valueNode = ParseValue();
	AST* lexpNode = NULL;

	switch(currentToken.type) {
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

	Advance();

	AST* rhs = ParseValue();	

	AddASTChild(lexpNode, valueNode);
	AddASTChild(lexpNode, rhs);

	return lexpNode;
}

AST* ParseIfStatement() {
	Match(TOKEN_IF);

	AST* ifNode = CreateASTNode(SEM_IF_STATEMENT, VALUE_EMPTY);

	AST* condition = ParseCondition();

	Match(TOKEN_THEN);

	AST* thenOp = ParseOperator();

	AddASTChild(ifNode, condition);
	AddASTChild(ifNode, thenOp);

	if(currentToken.type == TOKEN_ELSE) {
		Match(TOKEN_ELSE);
		AST* elseOp = ParseOperator();
		AddASTChild(ifNode, elseOp);
	}

	return ifNode;
}

AST* ParseFunctionCall() {
	Value* funcId = Match(TOKEN_ID);

	AST* funcNode = CreateASTNode(SEM_FUNCCALL, funcId);

	Match(TOKEN_LEFTBRACKET);
	AST* arglist = ParseArgumentList();
	Match(TOKEN_RIGHTBRACKET);

	AddASTChild(funcNode, arglist);

	return funcNode;
}

AST* ParseArgumentList() {
	AST* arglist = CreateASTNode(SEM_EMPTY, VALUE_EMPTY);	

	/* FIXME: At this time, ArgumentList is used both for calling
	 * and defining functions, which is wrong as it allows using
	 * constants as formal arguments of a function */
	while(currentToken.type != TOKEN_RIGHTBRACKET) {
		AST* argument = ParseValue();
		AddASTChild(arglist, argument);
		if(currentToken.type != TOKEN_RIGHTBRACKET)
			Match(TOKEN_COMMA);
	}

	return arglist;
}

AST* ParseExpression() {
	switch(currentToken.type) {
		case TOKEN_INTREAD:
			Match(TOKEN_INTREAD);
			return CreateASTNode(SEM_INTREAD, VALUE_EMPTY);
		case TOKEN_READ:
			Match(TOKEN_READ);
			return CreateASTNode(SEM_READ, VALUE_EMPTY);
		case TOKEN_ARRAY:
			return ParseArray();
		default:
			return ParseArithmeticalExpression();
	}

	assert(0);
	return NULL;
}

AST* ParseArithmeticalExpression() {
	AST* termNode = ParseTerm();

	if(currentToken.type == TOKEN_PLUS) {
		Match(TOKEN_PLUS);

		AST* exprNode = CreateASTNode(SEM_ADDITION, VALUE_EMPTY);
		AddASTChild(exprNode, termNode);
		AddASTChild(exprNode, ParseArithmeticalExpression());

		return exprNode;
	} else if(currentToken.type == TOKEN_MINUS) {
		Match(TOKEN_MINUS);

		AST* exprNode = CreateASTNode(SEM_SUBTRACTION, VALUE_EMPTY);
		AddASTChild(exprNode, termNode);
		AddASTChild(exprNode, ParseArithmeticalExpression());

		return exprNode;
	} else {
		return termNode;
	}
}

AST* ParseTerm() {
	AST* valueNode = ParseValue();

	if(currentToken.type == TOKEN_STAR) {
		Match(TOKEN_STAR);

		AST* exprNode = CreateASTNode(SEM_MULTIPLICATION, VALUE_EMPTY);
		AddASTChild(exprNode, valueNode);
		AddASTChild(exprNode, ParseTerm());

		return exprNode;
	} else if(currentToken.type == TOKEN_SLASH) {
		Match(TOKEN_SLASH);

		AST* exprNode = CreateASTNode(SEM_DIVISION, VALUE_EMPTY);
		AddASTChild(exprNode, valueNode);
		AddASTChild(exprNode, ParseTerm());

		return exprNode;
	} else {
		return valueNode;
	}
}

AST* ParseValue() {
	AST* node;
	if(currentToken.type == TOKEN_NUMBER || currentToken.type == TOKEN_STRING) {
		node =  CreateASTNode(SEM_CONSTANT, currentToken.value);
		Match(currentToken.type);
	} else {
		if(nextToken.type == TOKEN_LEFTBRACKET)
			return ParseFunctionCall();

		Value *id = Match(TOKEN_ID);

		/* XXX: Factor this out */
		if(currentToken.type == TOKEN_LEFT_SQUARE_BRACKET) {
			Match(TOKEN_LEFT_SQUARE_BRACKET);
			AST* size = ParseExpression();
			Match(TOKEN_RIGHT_SQUARE_BRACKET);

			node = CreateASTNode(SEM_INDEX,  id);
			AddASTChild(node, size);
		} else if(currentToken.type == TOKEN_DOT) { 
			Match(TOKEN_DOT);
			Value* field_name = Match(TOKEN_FIELD);
			node = CreateASTNode(SEM_FIELD, id);
			AddASTChild(node, CreateASTNode(SEM_CONSTANT, field_name));
		} else {
			node =  CreateASTNode(SEM_ID, id);
		}
	}

	return node;
}

AST* ParseReturn() {
	Match(TOKEN_RETURN);

	AST* node = CreateASTNode(SEM_RETURN, VALUE_EMPTY);
	AST* ret_expr = ParseExpression();

	AddASTChild(node, ret_expr);

	return node;
}

AST* ParseArray() {
	Match(TOKEN_ARRAY);
	Match(TOKEN_LEFT_SQUARE_BRACKET);

	/*Value* size = Match(TOKEN_NUMBER);*/
	AST* size_expr = ParseExpression();

	Match(TOKEN_RIGHT_SQUARE_BRACKET);

	AST* node = CreateASTNode(SEM_ARRAY, VALUE_EMPTY);
	AddASTChild(node, size_expr);

	return node;
}
