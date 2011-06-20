#ifndef IDTABLE_H
#define IDTABLE_H

#define MAX_ID_NUMBER		255

typedef struct {
	char* names[MAX_ID_NUMBER];
	int count;
} IDTable;

#endif /* IDTABLE_H */
