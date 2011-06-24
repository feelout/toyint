#ifndef MODULES_H
#define MODULES_H

#include "ast.h"
#include "idtable.h"

typedef struct Path {
	const char* path;
	struct Path* next;
} Path;

typedef struct {
	Path* paths;
} Modules;

void AddModulePath(const char* path);
AST* LoadModule(const char* name, IDTable* id_table);

#endif /* MODULES_H */
