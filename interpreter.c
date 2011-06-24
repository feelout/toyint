#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "interpreter.h"
#include "parser.h"

#define MAX_READ_STRING_SIZE	255

Value* InterpretExpression(AST* ast, Scope* scope, IDTable* id_table);

#define PERFORM_OP(op) \
	left = InterpretExpression(ast->child, scope, id_table); \
	Typecheck(left, TYPE_INTEGER); \
	right = InterpretExpression(ast->child->sibling, scope, id_table); \
	Typecheck(right, TYPE_INTEGER); \
	return CreateIntegralValue((left->v.integral) op (right->v.integral)) 

void FailWithInvalidArgumentNumber(Value* function) {
	fprintf(stderr, "Invalid number of arguments passed to function: expected %d\n", 
			function->v.function.argcount);
	exit(-1);
}


/* TODO: Do not mix coding styles */
/*Value* CallFunction(AST* ast, Scope* outer_scope, Value* owner) {*/
Value* CallFunction(Scope* outer_scope, Value* function, AST* arglist, Value* owner, IDTable* id_table) {
	int arg_num = 0;
	/*Value* function = GetValue(outer_scope, ast->value->v.integral); */
	Typecheck(function, TYPE_FUNCTION);

	Scope* local_scope = CreateScope();
	local_scope->parent = outer_scope;

	Value** arguments = (Value**)malloc(sizeof(Value*) * function->v.function.argcount);

	AST *arg;
	for(arg = arglist->child; arg; arg = arg->sibling) {
		if(arg_num == function->v.function.argcount) {
			FailWithInvalidArgumentNumber(function);
		}
		arguments[arg_num++] = InterpretExpression(arg, outer_scope, id_table);
	}

	if(arg_num < function->v.function.argcount) {
		FailWithInvalidArgumentNumber(function);
	}

	/* For each formal argument, set local values from @param arguments */
	for(arg_num = 0; arg_num < function->v.function.argcount; ++arg_num) {
		SetLocalValue(local_scope, function->v.function.arguments[arg_num], arguments[arg_num]);
	}

	SetLocalValue(local_scope, RETURN_VALUE_ID, NULL);
	SetLocalValue(local_scope, SELF_VALUE_ID, owner);
	InterpretAST(function->v.function.code, local_scope, id_table);

	/* Value* return_value = GetValue(local_scope, RETURN_VALUE_ID); */
	Value* return_value = local_scope->ids[RETURN_VALUE_ID]; /* Ignoring existence check */

	free(arguments);
	free(local_scope); /* XXX: When refcount is implemented, simple free call will not suffice */

	return return_value; /* TODO: Add new type NONE, as in Python, return it */
}

Value** ResolveIndex(AST* ast, Scope* scope, IDTable* id_table) {
	int id = ast->value->v.integral;
	Value* index = InterpretExpression(ast->child, scope, id_table);
	Typecheck(index, TYPE_INTEGER);

	Value* array = GetValue(scope, id);
	Typecheck(array, TYPE_ARRAY);

	if(index->v.integral >= array->v.array.size) {
		fprintf(stderr, "Array index out of bounds : %d out of %d\n", index->v.integral,
				array->v.array.size);
		exit(-1);
	}

	return &array->v.array.data[index->v.integral];
}

static void SetObjectField(AST* ast, Scope* scope, Value* value) {
	int id = ast->value->v.integral;
	Value* object = GetValue(scope, id);
	
	Typecheck(ast->child->value, TYPE_STRING);

	SetField(object, ast->child->value->v.string, value);
}

static Value* GetObjectField(AST* ast, Scope* scope) {
	int id = ast->value->v.integral;
	Value* object = GetValue(scope, id);

	Typecheck(ast->child->value, TYPE_STRING);

	return GetField(object,  ast->child->value->v.string);
}

static Value* InterpretFunctionDefinition(AST* ast) {
	int* func_args = (int*)malloc(sizeof(int) * MAX_FUNCTION_ARGUMENTS_COUNT);
	int arg_num = 0;

	AST* arg;

	for(arg = ast->child->child; arg; arg = arg->sibling) {
		func_args[arg_num++] = arg->value->v.integral;	
	}
	
	return CreateFunctionValue(func_args, arg_num, ast->child->sibling);
}

Value* InterpretExpression(AST* ast, Scope* scope, IDTable* id_table) {
	Value *left, *right, *value, *index, *size;
	int nvalue, id;
	char *read_buf;
	switch(ast->semantic) {
		case SEM_ID:
			return GetValue(scope, ast->value->v.integral);
		case SEM_CONSTANT:
			return ast->value;
		case SEM_INTREAD:
			printf("INPUT : ");
			scanf("%d", &nvalue);
			return CreateIntegralValue(nvalue);
		case SEM_READ:
			printf("INPUT : ");
			read_buf = (char*)malloc(sizeof(char) * MAX_READ_STRING_SIZE);
			scanf("%s", read_buf);
			return CreateStringValue(read_buf);
		case SEM_ARRAY:
			size = InterpretExpression(ast->child, scope, id_table);
			Typecheck(size, TYPE_INTEGER);
			return CreateArrayValue(size->v.integral);
		case SEM_OBJECT:
			return CreateObject();
		case SEM_INDEX:
			return *ResolveIndex(ast, scope, id_table);
		case SEM_FIELD:
			value = GetObjectField(ast, scope);
			if(!value) {
				fprintf(stderr, "Unknown field : %s\n", ast->child->value->v.string);
				exit(-1);
			}
			return value;
		case SEM_FUNCTION:
			return InterpretFunctionDefinition(ast);
		case SEM_FUNCCALL:
			return CallFunction(scope, GetValue(scope, ast->value->v.integral), ast->child, NULL, id_table);
		case SEM_NEW:
			{
				Value* object = CreateObject();
				Value* constructor = GetValue(scope, ast->child->value->v.integral);

				CallFunction(scope, constructor, ast->child->child, object, id_table);

				SetField(object, PROTOTYPE_VALUE_NAME, GetFieldGeneric(constructor, PROTOTYPE_VALUE_NAME, 0));

				return object;
			}
		case SEM_METHOD_CALL:
			return CallFunction(scope, GetObjectField(ast->child, scope), ast->child->sibling, 
					GetValue(scope, ast->child->value->v.integral), id_table);
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
			value = InterpretExpression(ast->child, scope, id_table);
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

void Assign(AST* ast, Scope* scope, int local, IDTable* id_table) {
	AST* lvalue = ast->child;

	int id = lvalue->value->v.integral;	
	Value* value = InterpretExpression(lvalue->sibling, scope, id_table);

	Value *index;
	Value **array_element;

	switch(lvalue->semantic) {
		case SEM_ID:
			if(local)
				SetLocalValue(scope, id, value);
			else
				SetValue(scope, id, value);
			break;
		case SEM_INDEX:
			/* TODO: Mutates value directly. Work out strict semantic
			 * for value access and modification!!!*/
			array_element = ResolveIndex(lvalue, scope, id_table);
			*array_element = value;
			break;
		case SEM_FIELD:
			SetObjectField(lvalue, scope, value);
			break;
		default:
			fprintf(stderr, "Invalid lvalue semantic : %d\n", lvalue->semantic);
			exit(-1);
			break;
	}
}

void InterpretStatement(AST* ast, Scope* scope, IDTable* id_table) {
	Value *value, *cond;
	char* value_repr;
	int* func_args;
	int arg_num;
	AST* arg;

	switch(ast->semantic) {
		case SEM_ASSIGNMENT:
		case SEM_LOCAL_ASSIGNMENT:
			Assign(ast, scope, ast->semantic == SEM_LOCAL_ASSIGNMENT, id_table);
			break;
		case SEM_WHILE_CYCLE:
			while(InterpretExpression(ast->child, scope, id_table)->v.integral) {
				InterpretStatement(ast->child->sibling, scope, id_table);
			}
			break;
		case SEM_IF_STATEMENT:
			cond = InterpretExpression(ast->child, scope, id_table);
			if(cond->v.integral) {
				InterpretStatement(ast->child->sibling, scope, id_table);
			} else if(ast->child->sibling->sibling) {
				InterpretStatement(ast->child->sibling->sibling, scope, id_table);
			}
			break;
		case SEM_PRINT:
			value = InterpretExpression(ast->child, scope, id_table);
			value_repr = ValueToString(value);
			printf("OUTPUT: %s\n", value_repr);
			free(value_repr);
			break;
		case SEM_RETURN:
			SetLocalValue(scope, RETURN_VALUE_ID, InterpretExpression(ast->child, scope, id_table));
			break;
		case SEM_FUNCTION:
			SetLocalValue(scope, ast->value->v.integral, InterpretFunctionDefinition(ast));
			break;
		case SEM_FUNCCALL:
			CallFunction(scope, GetValue(scope, ast->value->v.integral), ast->child, NULL, id_table);
			break;
		case SEM_METHOD_CALL:
			CallFunction(scope, GetObjectField(ast->child, scope), ast->child->sibling, ast->child->value, id_table);
			break;
		case SEM_EMPTY:
			InterpretAST(ast->child, scope, id_table);
			break;
		case SEM_INCLUDE:
			{
				Value* file_name = ast->value;
				Typecheck(file_name, TYPE_STRING);
				InterpretAST(ParseFile(file_name->v.string, id_table), scope, id_table);
			}
			break;
		default:
			fprintf(stderr, "Interpreting error : unexpected statement semantic : %s\n", semanticToString[ast->semantic]);
			break;
	}
}

void InterpretAST(AST* ast, Scope* scope, IDTable* id_table) {
	AST* currentAST;
	for(currentAST = ast; currentAST; currentAST = currentAST->sibling) {
		InterpretStatement(currentAST, scope, id_table);
	}
}
