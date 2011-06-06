#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "parser.h"
#include "lexer.h"
#include "ast.h"

LexerState lex;
Token currentToken;

void FailWithUnexpectedToken(int got, int needed) {
	fprintf(stderr, "Unexpected token: \"%s\", needed \"%s\"\n", tokenName[got], tokenName[needed]);
	exit(-1);
}

void FailWithParsingError(char* msg) {
	fprintf(stderr, "Parsing error : %s\n", msg);
	exit(-1);
}

void Advance() {
	currentToken = GetNextToken(&lex);
	/*printf("Advance : next token is %d\n", currentToken.type);*/
}

int Match(int expectedToken) {
	if(currentToken.type != expectedToken) {
		FailWithUnexpectedToken(currentToken.type, expectedToken);
	}

	Advance();

	return currentToken.value;
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

AST* ParseFile(char* filename) {
	StartLexer(&lex, filename);
	return ParseProgram();
}

AST* ParseProgram() {
	Advance();
	return ParseBlock();
}

/* Block does not create new node */
AST* ParseBlock() {
	Match(TOKEN_BEGIN);

	AST* oplist = ParseOperatorList();

	if(!oplist) {
		/* XXX: Delete this, as ParseOperatorList always creates at least
		 * an empty AST node */
		FailWithParsingError("expected operator list");
	}

	Match(TOKEN_END);

	return oplist;
}

AST* ParseOperatorList() {
	AST *operators = CreateASTNode(SEM_EMPTY, VALUE_EMPTY);

	while(currentToken.type != TOKEN_END) {
		AST *op = ParseOperator();
		AddASTChild(operators, op);
		Match(TOKEN_SEMICOLON);
	}

	return operators;
}

AST* ParseOperator() {
	switch(currentToken.type) {
		case TOKEN_BEGIN:
			return ParseBlock();
		case TOKEN_ID:
			return ParseAssignment();
		case TOKEN_WHILE:
			return ParseWhileCycle();
		case TOKEN_IF:
			return ParseIfStatement();
		case TOKEN_CALL:
			return ParseFunctionCall();
	}

	FailWithUnexpectedToken(currentToken.type, TOKEN_ID);

	return NULL;
}

AST* ParseAssignment() {
	int id;

	id = Match(TOKEN_ID);	
	Match(TOKEN_ASSIGNMENT);

	AST *idNode = CreateASTNode(SEM_ID, id);
	AST *expr = ParseExpression();

	AST *assignment = CreateASTNode(SEM_ASSIGNMENT, VALUE_EMPTY);

	AddASTChild(assignment, idNode);
	AddASTChild(assignment, expr);

	return assignment;
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
	/* XXX: Add conditions */
	if(currentToken.type == TOKEN_NOT) {
		AST* notCond = CreateASTNode(SEM_NOT, VALUE_EMPTY);
		Match(TOKEN_NOT);

		AddASTChild(notCond, ParseLogicalExpression());

		return notCond;
	} else {
		AST* lexp = ParseLogicalExpression();
		AST* rhs;

		switch(currentToken.type) {
			/* XXX Refactor */
			case TOKEN_AND:
				Match(TOKEN_AND);
				rhs = ParseCondition();
				AST *andNode = CreateASTNode(SEM_AND, VALUE_EMPTY);
				AddASTChild(andNode, lexp);
				AddASTChild(andNode, rhs);

				return andNode;
			case TOKEN_OR:
				Match(TOKEN_OR);
				rhs = ParseCondition();
				AST *orNode = CreateASTNode(SEM_OR, VALUE_EMPTY);
				AddASTChild(orNode, lexp);
				AddASTChild(orNode, rhs);

				return orNode;
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
	Match(TOKEN_CALL);

	int funcId = Match(TOKEN_ID);

	AST* funcNode = CreateASTNode(SEM_FUNCCALL, funcId);

	Match(TOKEN_LEFTBRACKET);
	AST* arglist = ParseArgumentList();
	Match(TOKEN_RIGHTBRACKET);

	AddASTChild(funcNode, arglist);

	return funcNode;
}

AST* ParseArgumentList() {
	AST* arglist = CreateASTNode(SEM_EMPTY, VALUE_EMPTY);	

	while(currentToken.type != TOKEN_RIGHTBRACKET) {
		AST* argument = ParseValue();
		AddASTChild(arglist, argument);
	}

	return arglist;
}

AST* ParseExpression() {
	if(currentToken.type == TOKEN_CALL) {
		// TODO: Parse function call
	} else {
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
	} else if(currentToken.type == TOKEN_MINUS) {
		Match(TOKEN_MINUS);

		AST* exprNode = CreateASTNode(SEM_DIVISION, VALUE_EMPTY);
		AddASTChild(exprNode, valueNode);
		AddASTChild(exprNode, ParseTerm());

		return exprNode;
	} else {
		return valueNode;
	}
}

AST* ParseValue() {
	if(currentToken.type == TOKEN_NUMBER) {
		Match(TOKEN_NUMBER);

		return CreateASTNode(SEM_CONSTANT, currentToken.value);
	} else {
		Match(TOKEN_ID);

		return CreateASTNode(SEM_ID, currentToken.value);
	}
}
