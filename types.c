#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "ast.h"

#define BUFFER_SIZE 256
#define FIELDS_TABLE_INITIAL_SIZE	20

void Typecheck(Value* value, enum Type type) {
	if(value->type != type) {
		fprintf(stderr, "Typecheck failed: expected %d, got %d\n", type, value->type);
		exit(-1);
	}
}
char* ValueToString(Value* value) {
	char *buffer;
	int ret; 

	/*int ind, left;
	char *write_ptr */

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
			/*left = BUFFER_SIZE-1;
			write_ptr = buffer;
			for(ind = 0; ind < value->v.array.size; ++ind) {
				ret = sprintf(write_ptr, "%d ", value->v.array.data[ind]);
				if(ret < 0) {
					fprintf(stderr, "Too long array to output, maximum buffer size is %d\n",
							BUFFER_SIZE);
					exit(-1);
				}
				write_ptr += ret;
			}*/
			strcpy(buffer, "[array]");
			break;
		case TYPE_FUNCTION:
			strcpy(buffer, "[function]");
			break;
		default:
			fprintf(stderr, "Invalid data type : %d\n", value->type);
			break;
	}

	return buffer;
}

Value* CreateValue() {
	Value* value = (Value*)malloc(sizeof(Value));

	value->fields = (KeyValue**)malloc(sizeof(KeyValue*) * FIELDS_TABLE_INITIAL_SIZE);
	value->fields_table_size = FIELDS_TABLE_INITIAL_SIZE;
	value->fields_num = 0;

	memset(value->fields, 0, sizeof(KeyValue*) * FIELDS_TABLE_INITIAL_SIZE);

	return value;
}

Value* CreateObject() {
	return CreateValue();
}

Value* CreateIntegralValue(int nvalue) {
	Value* value = CreateValue();

	value->type = TYPE_INTEGER;
	value->v.integral = nvalue;

	return value;
}

Value* CreateStringValue(char* string) {
	Value* value = CreateValue();

	value->type = TYPE_STRING;
	value->v.string = string;

	return value;
}

Value* CreateFunctionValue(int* arguments, int argcount, AST* code) {
	Value* value = CreateValue();

	value->type = TYPE_FUNCTION;
	value->v.function.argcount = argcount;
	value->v.function.arguments = arguments;
	value->v.function.code = code;

	return value;
}

Value* CreateArrayValue(int size) {
	Value* value = CreateValue();

	value->type = TYPE_ARRAY;
	value->v.array.size = size;
	value->v.array.data = (Value**)malloc(sizeof(Value*) * size);

	SetField(value, "length", CreateIntegralValue(size));

	return value;
}

int HashString(const char* str, int table_size) {
	int len = strlen(str);
	int hash = 0;
	int i;
	for(i = 0; i < len; ++i) {
		hash = (hash * 31 + str[i]) % table_size;
	}

	return hash;
}

int GetFieldIndex(Value* value, const char* field_key) {
	/* TODO: Check for load factor and free space */
	int hash = HashString(field_key, value->fields_table_size);

	int index = hash;
	while(value->fields[index] && strcmp(value->fields[index]->key, field_key) != 0) {
		++index;
		if(index >= value->fields_table_size)
			index = 0;
	}

	return index;
}

static void Rehash(Value* value) {
	int old_size = value->fields_table_size;
	value->fields_table_size = old_size * 2;

	KeyValue** old_fields = value->fields;

	value->fields = (KeyValue**)malloc(sizeof(KeyValue*) * value->fields_table_size);
	memset(value->fields, 0, sizeof(KeyValue*) * value->fields_table_size);
	value->fields_num = 0;

	int i, hash;
	KeyValue* kv;
	for(i = 0; i < old_size; ++i) {
		kv = old_fields[i];

		if(kv) {
			SetField(value, kv->key, kv->value);
			free(kv);
		}
	}

	free(old_fields);
}

void SetField(Value* value, const char* field_key, Value* field_value) {
	/* Rehash if the load factor is greater that 0.75 (XXX: Use named constant?) */
	if(4 * value->fields_num > 3 * value->fields_table_size) {
		Rehash(value);
	}
	int index = GetFieldIndex(value, field_key);

	KeyValue* entry = (KeyValue*)malloc(sizeof(KeyValue));

	char *key_copy = (char*)malloc(sizeof(char) * (strlen(field_key) + 1));
	strcpy(key_copy, field_key);
	entry->key = key_copy;

	entry->value = field_value;

	if(!value->fields[index])
		++value->fields_num;
	else if(!field_value)
		--value->fields_num;
	else {
		free(value->fields[index]->key);
		free(value->fields[index]);
	}

	value->fields[index] = entry;
}

Value* GetFieldGeneric(Value* value, const char* field_key, int check_prototype) {
	int index = GetFieldIndex(value, field_key);

	if(value->fields[index])
		return value->fields[index]->value;

	if(check_prototype) {
		Value* prototype = GetFieldGeneric(value, PROTOTYPE_VALUE_NAME, 0);

		if(prototype)
			return GetFieldGeneric(prototype, field_key, check_prototype);
	}

	return NULL;
}

Value* GetField(Value* value, const char* field_key) {
	return GetFieldGeneric(value, field_key, 1);
}
