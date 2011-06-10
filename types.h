#ifndef TYPES_H
#define TYPES_H

/* #include "ast.h" */

struct AST;

enum Type {
	TYPE_INTEGER = 0, TYPE_STRING, TYPE_ARRAY, TYPE_FUNCTION,
};

#define MAX_FUNCTION_ARGUMENTS_COUNT	10

typedef struct {
	enum Type type;
	union {
		int integral;
		char* string;
		struct {
			int *data;
			int size;
		} array;
		struct {
			int argcount;
			int *arguments;
			struct AST* code;
		} function;
	} v;
} Value;

void Typecheck(Value* value, enum Type type);
char* ValueToString(Value* value);
Value* CreateIntegralValue(int nvalue);
Value* CreateStringValue(char* string);
Value* CreateFunctionValue(int* arguments, int argcount, struct AST* code);

#endif // TYPES_H
