#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "modules.h"
#include "parser.h"

#define MODULE_FULL_PATH_MAX_LENGTH		250

Modules modules;

void AddModulePath(const char* path) {
	Path* node = modules.paths;

	Path* new_path = (Path*)malloc(sizeof(Path));
	new_path->path = strdup(path);
	new_path->next = NULL;

	if(!node) {
		modules.paths = new_path;
		return;
	}

	while(node->next) node = node->next;

	node->next = new_path;
}

int FileExists(const char* name) {
	FILE* file = fopen(name, "r");

	if(!file)
		return 0;

	fclose(file);

	return 1;
}

AST* LoadModule(const char* name, IDTable* id_table) {
	Path* node;

	char module_file_name[MODULE_FULL_PATH_MAX_LENGTH]; 
	char buffer[MODULE_FULL_PATH_MAX_LENGTH];

	int found = 0;

	for(node = modules.paths; node; node = node->next) {
		strcpy(buffer, node->path);
		strcat(buffer, name);

		if(FileExists(buffer)) {
			if(found) {
				fprintf(stderr, "Ambiguous module name : %s\n", name);
				fprintf(stderr, "Found at %s\n", buffer);
				fprintf(stderr, "Found at %s\n", module_file_name);
				exit(-1);
			}

			strcpy(module_file_name, buffer);
			found = 1;
		}
	}

	if(!found) {
		fprintf(stderr, "Module not found : %s\n", name);
		exit(-1);
	}

	return ParseFile(module_file_name, id_table);
}
