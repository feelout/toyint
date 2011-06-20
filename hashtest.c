#include <stdio.h>
#include <stdlib.h>
#include "types.h"

void DumpHash(Value* object) {
	int i;
	char* repr;

	for(i = 0; i < object->fields_table_size; ++i) {
		KeyValue* kv = object->fields[i];
		if(kv) {
			repr = ValueToString(kv->value);
			printf("%s\t%s\n", kv->key, repr);
			free(repr);
		}
	}
}

int main(int argc, char* argv[]) {
	char buffer[100];
	int value;
	char cmd;
	Value* object = CreateIntegralValue(0);
	Value* field;

	while(1) {
		printf("Enter (q|g|p|h|d) : ");
		scanf("%c", &cmd);
		switch(cmd) {
			case 'q':
				return 0;
			case 'g':
				printf("Getting\n");
				scanf("%s", buffer);
				field = GetField(object, buffer);
				if(!field)
					printf("Value not found\n");
				else
					printf("Value : %d\n", field->v.integral);
				break;
			case 'p':
				printf("Putting\n");
				scanf("%s %d", buffer, &value);
				SetField(object, buffer, CreateIntegralValue(value));
				break;
			case 'h':
				printf("Hashing\n");
				scanf("%s", buffer);
				printf("Hash of %s : %d\n", buffer, HashString(buffer, object->fields_table_size));
				break;
			case 'd':
				DumpHash(object);
				break;
			default:
				printf("Unknown command : \"%c\"\n", cmd);
				break;
		}
		getc(stdin);
	}

	printf("\n");
	return 0;
}
