#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "modules.h"
#include "parser.h"

#include <dlfcn.h>

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

char* FindModule(const char* name) {
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

	return strdup(module_file_name);
}

AST* IncludeModule(const char* name, IDTable* id_table) {
	char* module_file_name = FindModule(name);

	printf("Found module %s at %s\n", name, module_file_name);

	return ParseFile(module_file_name, id_table);
}

/** XXX: Unload modules later */
void LoadModule(const char* name, IDTable* id_table, Scope* scope) {
	char* module_file_name = FindModule(name);

	void* handle = dlopen(module_file_name, RTLD_LAZY);
	if(!handle) {
		fprintf(stderr, "Failed to load module %s: %s\n", module_file_name, dlerror());
		exit(-1);
	}

	ModuleInitFunc init_func = dlsym(handle, "Init");

	char* error = dlerror();

	if(error) {
		fprintf(stderr, "Failed to load module %s: %s\n", module_file_name, error);
		exit(-1);
	}

	init_func(id_table, scope);
}
