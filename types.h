#ifndef TYPES_H
#define TYPES_H

/* #include "ast.h" */

struct AST;
struct Scope;

enum Type {
	TYPE_INTEGER = 0, TYPE_STRING, TYPE_ARRAY, TYPE_FUNCTION, TYPE_OBJECT, 
		TYPE_FOREIGN_FUNCTION
};

#define MAX_FUNCTION_ARGUMENTS_COUNT	10
#define PROTOTYPE_VALUE_NAME			"prototype"

typedef void (*ForeignFunction)(struct Scope*);

typedef struct Value {
	enum Type type;
	union {
		int integral;
		char* string;
		struct {
			struct Value **data;
			int size;
		} array;
		struct {
			int argcount;
			int *arguments;
			struct AST* code;
		} function;
	} v;
	struct KeyValue** fields;
	int fields_table_size;
	int fields_num;
} Value;

typedef struct KeyValue {
	char* key;
	Value* value;
} KeyValue;

void Typecheck(Value* value, enum Type type);
char* ValueToString(Value* value);
Value* CreateIntegralValue(int nvalue);
Value* CreateStringValue(char* string);
Value* CreateFunctionValue(int* arguments, int argcount, struct AST* code);
Value* CreateArrayValue(int size);
Value* CreateObject();

int HashString(const char* str, int table_size);

void SetField(Value* value, const char* field_key, Value* field_value);
Value* GetField(Value* value, const char* field_key);
Value* GetFieldGeneric(Value* value, const char* field_key, int check_prototype);

#endif // TYPES_H
