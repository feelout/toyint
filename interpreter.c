#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "interpreter.h"

#define MAX_READ_STRING_SIZE	255

#define PERFORM_OP(op) \
	left = InterpretExpression(ast->child, scope); \
	Typecheck(left, TYPE_INTEGER); \
	right = InterpretExpression(ast->child->sibling, scope); \
	Typecheck(right, TYPE_INTEGER); \
	return CreateIntegralValue((left->v.integral) op (right->v.integral)) 

/* TODO: Do not mix coding styles */
Value* CallFunction(AST* ast, Scope* outer_scope) {
	int arg_num = 0;
	Value* function = GetValue(outer_scope, ast->value->v.integral);
	Typecheck(function, TYPE_FUNCTION);

	Scope* local_scope = CreateScope();
	local_scope->parent = outer_scope;

	Value** arguments = (Value**)malloc(sizeof(Value*) * ast->value->v.function.argcount);

	AST *arg;
	/* TODO: Check for extra arguments */
	for(arg = ast->child->child; arg; arg = arg->sibling) {
		arguments[arg_num++] = arg->value;	
	}

	/* For each formal argument, set local values from @param arguments */
	for(arg_num = 0; arg_num < function->v.function.argcount; ++arg_num) {
		SetLocalValue(local_scope, function->v.function.arguments[arg_num], arguments[arg_num]);
	}

	SetLocalValue(local_scope, RETURN_VALUE_ID, NULL);
	InterpretAST(function->v.function.code, local_scope);

	Value* return_value = GetValue(local_scope, RETURN_VALUE_ID);

	free(arguments);
	free(local_scope);

	/* TODO: Set return value */
	return return_value; /* TODO: Add new type NONE, as in Python, return it */
}
/* NOTE: Return values can be passed in local scope,
 * which will be additionally examined by InterpretExpression
 * (because we are ignoring return value in InterpretStatement)
 */
Value* InterpretExpression(AST* ast, Scope* scope) {
	Value *left, *right, *value;
	int nvalue;
	char *read_buf;
	switch(ast->semantic) {
		case SEM_ID:
			return GetValue(scope, ast->value->v.integral);
		case SEM_CONSTANT:
			return ast->value;
		case SEM_INTREAD: // TODO: Add another read for strings
			printf("INPUT : ");
			scanf("%d", &nvalue);
			return CreateIntegralValue(nvalue);
		case SEM_READ:
			printf("INPUT : ");
			read_buf = (char*)malloc(sizeof(char) * MAX_READ_STRING_SIZE);
			scanf("%s", read_buf);
			return CreateStringValue(read_buf);
		case SEM_FUNCCALL:
			return CallFunction(ast, scope);
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
	int* func_args;
	int arg_num;
	AST* arg;

	switch(ast->semantic) {
		case SEM_ASSIGNMENT:
			id = ast->child->value->v.integral;
			value = InterpretExpression(ast->child->sibling, scope);
			SetValue(scope, id, value);
			break;
		case SEM_LOCAL_ASSIGNMENT:
			id = ast->child->value->v.integral;
			value = InterpretExpression(ast->child->sibling, scope);
			SetLocalValue(scope, id, value);
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
		case SEM_RETURN:
			SetLocalValue(scope, RETURN_VALUE_ID, InterpretExpression(ast->child, scope));
			break;
		case SEM_FUNCTION:
			func_args = (int*)malloc(sizeof(int) * MAX_FUNCTION_ARGUMENTS_COUNT);
			arg_num = 0;
			for(arg = ast->child->child; arg; arg = arg->sibling) {
				func_args[arg_num++] = arg->value->v.integral;	
			}
			SetLocalValue(scope, ast->value->v.integral, CreateFunctionValue(func_args, arg_num, ast->child->sibling));
			break;
		case SEM_FUNCCALL:
			CallFunction(ast, scope);
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
