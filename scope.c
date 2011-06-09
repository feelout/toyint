#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scope.h"

Scope* CreateScope() {
	Scope *scope =  (Scope*)malloc(sizeof(Scope));
	memset(scope->ids, 0, sizeof(Value*) * MAX_IDS);
	//memset(scope->contains, 0, sizeof(int) * MAX_IDS);
	scope->parent = NULL;

	return scope;
}

int ExistsInScope(Scope *scope, int id) {
	/*if(scope->contains[id]) {
		return 1;
	}*/
	if(scope->ids[id])
		return 1;

	return scope->parent ? ExistsInScope(scope->parent, id) : 0;
}

Value* GetValue(Scope *scope, int id) {
	if(scope->ids[id]) {
		return scope->ids[id];
	}

	return scope->parent ? GetValue(scope->parent, id) : NULL;
}

void SetValue(Scope *scope, int id, Value* value) {
	if(ExistsInScope(scope, id)) {
		while(!scope->ids[id]) {
			scope = scope->parent;
		}
	}

	scope->ids[id] = value;
}
