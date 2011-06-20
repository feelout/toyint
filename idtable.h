#ifndef IDTABLE_H
#define IDTABLE_H

#define MAX_ID_NUMBER		255
#define RETURN_VALUE_ID	0
#define SELF_VALUE_ID 1

typedef struct {
	char* names[MAX_ID_NUMBER];
	int count;
} IDTable;

IDTable* CreateIDTable();

/* Takes id name and returns it's number in id table, adding
 * it there if necessary */
int GetIDIndex(IDTable* table, char* id);

#endif /* IDTABLE_H */
