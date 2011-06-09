#ifndef TYPES_H
#define TYPES_H

enum Type {
	TYPE_INTEGER = 0, TYPE_STRING, TYPE_ARRAY, 
};

typedef struct {
	enum Type type;
	union {
		int integral;
		char* string;
		struct {
			int *data;
			int size;
		} array;
	} v;
} Value;

void Typecheck(Value* value, enum Type type);
char* ValueToString(Value* value);
Value* CreateIntegralValue(int nvalue);
Value* CreateStringValue(char* string);

#endif // TYPES_H
