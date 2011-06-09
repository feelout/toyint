#include <stdio.h>
#include <stdlib.h>
#include "interpreter.h"

#define PERFORM_OP(op) \
	left = InterpretExpression(ast->child, scope); \
	Typecheck(left, TYPE_INTEGER); \
	right = InterpretExpression(ast->child->sibling, scope); \
	Typecheck(right, TYPE_INTEGER); \
	return CreateIntegralValue((left->v.integral) op (right->v.integral)) 

Value* InterpretExpression(AST* ast, Scope* scope) {
	Value *left, *right, *value;
	int nvalue;
	switch(ast->semantic) {
		case SEM_ID:
			return GetValue(scope, ast->value->v.integral);
		case SEM_CONSTANT:
			return ast->value;
		case SEM_READ: // TODO: Add another read for strings
			printf("INPUT : ");
			scanf("%d", &nvalue);
			return CreateIntegralValue(nvalue);
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
			Typecheck(value, TYPE_INTEGER);
			return CreateIntegralValue(!value->v.integral);
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
	int id; 
	Value *value, *cond;
	char* value_repr;

	switch(ast->semantic) {
		case SEM_ASSIGNMENT:
			id = ast->child->value->v.integral;
			value = InterpretExpression(ast->child->sibling, scope);
			SetValue(scope, id, value);
			break;
		case SEM_WHILE_CYCLE:
			while(InterpretExpression(ast->child, scope)->v.integral) {
				InterpretStatement(ast->child->sibling, scope);
			}
			break;
		case SEM_IF_STATEMENT:
			cond = InterpretExpression(ast->child, scope);
			if(cond->v.integral) {
				InterpretStatement(ast->child->sibling, scope);
			} else if(ast->child->sibling->sibling) {
				InterpretStatement(ast->child->sibling->sibling, scope);
			}
			break;
		case SEM_PRINT:
			value = InterpretExpression(ast->child, scope);
			value_repr = ValueToString(value);
			printf("OUTPUT: %s\n", value_repr);
			free(value_repr);
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
