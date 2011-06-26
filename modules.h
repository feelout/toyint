#ifndef MODULES_H
#define MODULES_H

#include "ast.h"
#include "idtable.h"
#include "scope.h"

/** TODO: Add cache */
typedef struct Path {
	const char* path;
	struct Path* next;
} Path;

typedef struct {
	Path* paths;
} Modules;

typedef void (*ModuleInitFunc)(IDTable*, Scope*);

void AddModulePath(const char* path);
AST* IncludeModule(const char* name, IDTable* id_table);
void LoadModule(const char* name, IDTable* id_table, Scope* scope);

#endif /* MODULES_H */
