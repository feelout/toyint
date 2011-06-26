#include "modules.h"
#include "types.h"

void Init(IDTable* table, Scope* scope) {
	int id = GetIDIndex(table, "foreign_value");	

	SetValue(scope, id, CreateIntegralValue(42));
}
