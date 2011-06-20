#include <string.h>
#include <stdlib.h>
#include "idtable.h"

IDTable* CreateIDTable() {
	IDTable* table = (IDTable*)malloc(sizeof(IDTable));

	memset(table->names, 0, sizeof(char*) * MAX_ID_NUMBER);

	/* Should really just be determined by runtime */
	table->names[RETURN_VALUE_ID] = "__retvalue__";
	table->names[SELF_VALUE_ID] = "self"; 
	table->count = 2; /* XXX: Misleading name, actually number of the last id */

	return table;
}

int GetIDIndex(IDTable* table, char* id) {
	int n;
	for(n = 0; n < table->count; ++n) {
		if(strcmp(id, table->names[n]) == 0)
			return n;
	}

	/* XXX: strdup()! */
	char *id_name = (char*)malloc(sizeof(char) * (strlen(id) + 1));
	strcpy(id_name, id);

	table->names[table->count] = id_name;

	return table->count++;
}
