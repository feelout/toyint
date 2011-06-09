#ifndef AST_H
#define AST_H

#include "types.h"

enum Semantic {
	SEM_ASSIGNMENT = 0, SEM_ID, SEM_CONSTANT, 
	SEM_ADDITION, SEM_SUBTRACTION, SEM_MULTIPLICATION, SEM_DIVISION,
	SEM_WHILE_CYCLE, SEM_IF_STATEMENT, SEM_FUNCCALL, SEM_AND,
	SEM_OR, SEM_NOT, SEM_LT, SEM_EQ, SEM_GT, SEM_LTE, SEM_GTE, SEM_PRINT, 
	SEM_READ, SEM_EMPTY
};

#define VALUE_EMPTY NULL

typedef struct AST {
	int semantic;	
	Value* value; 

	struct AST *child;
	struct AST *sibling;
	struct AST *parent;
} AST;

extern char* semanticToString[];

AST* CreateASTNode(int semantic, Value* value);
void AddASTChild(AST* parent, AST* child);
void DumpAST(AST *ast);

#endif /* AST_H */
