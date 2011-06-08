#include <stdio.h>
#include "interpreter.h"

#define PERFORM_OP(op) \
	left = InterpretExpression(ast->child, scope); \
	right = InterpretExpression(ast->child->sibling, scope); \
	return left op right

int InterpretExpression(AST* ast, Scope* scope) {
	int left, right, value;
	switch(ast->semantic) {
		case SEM_ID:
			return GetValue(scope, ast->value);
		case SEM_CONSTANT:
			return ast->value;
		case SEM_READ:
			printf("INPUT : ");
			scanf("%d", &value);
			return value;
		case SEM_ADDITION:
			PERFORM_OP(+);
		case SEM_SUBTRACTION:
			PERFORM_OP(-);
		case SEM_MULTIPLICATION:
			PERFORM_OP(*);
		case SEM_DIVISION:
			PERFORM_OP(/);
		case SEM_AND:
			PERFORM_OP(&&);
		case SEM_OR:
			PERFORM_OP(||);
		case SEM_NOT:
			value = InterpretExpression(ast->child, scope);
			return !value;
		case SEM_LT:
			PERFORM_OP(<);
		case SEM_EQ:
			PERFORM_OP(==);
		case SEM_GT:
			PERFORM_OP(>);
		case SEM_LTE:
			PERFORM_OP(<=);
		case SEM_GTE:
			PERFORM_OP(>=);
		default:
			fprintf(stderr, "Interpreting error : unexpected expression semantic : %s\n", semanticToString[ast->semantic]);
			return 0;
	}
}

void InterpretStatement(AST* ast, Scope* scope) {
	int id, value, cond;
	switch(ast->semantic) {
		case SEM_ASSIGNMENT:
			id = ast->child->value;
			value = InterpretExpression(ast->child->sibling, scope);
			SetValue(scope, id, value);
			break;
		case SEM_WHILE_CYCLE:
			while(InterpretExpression(ast->child, scope)) {
				InterpretStatement(ast->child->sibling, scope);
			}
			break;
		case SEM_IF_STATEMENT:
			cond = InterpretExpression(ast->child, scope);
			if(cond) {
				InterpretStatement(ast->child->sibling, scope);
			} else if(ast->child->sibling->sibling) {
				InterpretStatement(ast->child->sibling->sibling, scope);
			}
			break;
		case SEM_PRINT:
			value = InterpretExpression(ast->child, scope);
			printf("OUTPUT: %d\n", value);
			break;
		case SEM_EMPTY:
			InterpretAST(ast->child, scope);
			break;
		default:
			fprintf(stderr, "Interpreting error : unexpected statement semantic : %s\n", semanticToString[ast->semantic]);
			break;
	}
}

void InterpretAST(AST* ast, Scope* scope) {
	AST* currentAST;
	for(currentAST = ast; currentAST; currentAST = currentAST->sibling) {
		InterpretStatement(currentAST, scope);
	}
}
