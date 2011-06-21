#ifndef AST_H
#define AST_H

#include "types.h"

enum Semantic {
	SEM_ASSIGNMENT = 0, SEM_ID, SEM_CONSTANT, SEM_ADDITION,		/* 0..3 */
	SEM_SUBTRACTION, SEM_MULTIPLICATION, SEM_DIVISION,			/* 4..6 */
	SEM_WHILE_CYCLE, SEM_IF_STATEMENT, SEM_FUNCCALL, SEM_AND,	/* 7..10 */
	SEM_OR, SEM_NOT, SEM_LT, SEM_EQ, SEM_GT, SEM_LTE, SEM_GTE, 	/* 11..17 */
	SEM_PRINT, SEM_INTREAD, SEM_READ, SEM_FUNCTION, 			/* 18..22 */
	SEM_LOCAL_ASSIGNMENT, SEM_RETURN, SEM_ARRAY, SEM_INDEX,		/* 23..26 */
	SEM_FIELD, SEM_METHOD_CALL, SEM_OBJECT, SEM_INCLUDE,		/* 27..30 */
	SEM_EMPTY,			
};

#define VALUE_EMPTY NULL

typedef struct AST {
	enum Semantic semantic;	
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
