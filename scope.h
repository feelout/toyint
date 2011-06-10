#ifndef SCOPE_H
#define SCOPE_H

#include "types.h"

#define MAX_IDS 255

/* NB: Do not traverse all stack to find a binding, copy whole
 * scope for now. And ffs, write an associative container!
 */

typedef struct Scope{
	Value* ids[MAX_IDS];
	struct Scope *parent;
} Scope;

Scope* CreateScope();
int ExistsInScope(Scope *scope, int id);
Value* GetValue(Scope *scope, int id);
void SetValue(Scope *scope, int id, Value* value);
void SetLocalValue(Scope *scope, int id, Value* value);

#endif /* SCOPE_H */
