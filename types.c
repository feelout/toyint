#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "ast.h"

#define BUFFER_SIZE 256

void Typecheck(Value* value, enum Type type) {
	if(value->type != type) {
		fprintf(stderr, "Typecheck failed: expected %d, got %d\n", type, value->type);
		exit(-1);
	}
}
char* ValueToString(Value* value) {
	char *buffer, *write_ptr;
	int ret, ind, left;

	buffer = (char*)malloc(sizeof(char) * BUFFER_SIZE);

	switch(value->type) {
		case TYPE_INTEGER:
			ret = sprintf(buffer, "%d", value->v.integral);
			if(ret < 0) {
				fprintf(stderr, "Error while converting integer %d to a string\n", value->v.integral);
				exit(-1);
			}
			break;
		case TYPE_STRING:
			if(strlen(value->v.string) >= BUFFER_SIZE) {
				fprintf(stderr, "Too long string to output: %s, maximum buffer size is %d\n",
						value->v.string, BUFFER_SIZE);
				exit(-1);
			}
			strcpy(buffer, value->v.string);
			break;
		case TYPE_ARRAY:
			left = BUFFER_SIZE-1;
			write_ptr = buffer;
			for(ind = 0; ind < value->v.array.size; ++ind) {
				ret = sprintf(write_ptr, "%d ", value->v.array.data[ind]);
				if(ret < 0) {
					fprintf(stderr, "Too long array to output, maximum buffer size is %d\n",
							BUFFER_SIZE);
					exit(-1);
				}
				write_ptr += ret;
			}
			break;
		default:
			fprintf(stderr, "Invalid data type : %d\n", value->type);
			break;
	}

	return buffer;
}

Value* CreateIntegralValue(int nvalue) {
	Value* value = (Value*)malloc(sizeof(Value));

	value->type = TYPE_INTEGER;
	value->v.integral = nvalue;

	return value;
}

Value* CreateStringValue(char* string) {
	Value* value = (Value*)malloc(sizeof(Value));

	value->type = TYPE_STRING;
	value->v.string = string;

	return value;
}

Value* CreateFunctionValue(int* arguments, int argcount, AST* code) {
	Value* value = (Value*)malloc(sizeof(Value));

	value->type = TYPE_FUNCTION;
	value->v.function.argcount = argcount;
	value->v.function.arguments = arguments;
	value->v.function.code = code;

	return value;
}
