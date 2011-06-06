#ifndef AST_H
#define AST_H

enum Semantic {SEM_ASSIGNMENT = 0, SEM_ID, SEM_CONSTANT, 
	SEM_ADDITION, SEM_SUBTRACTION, SEM_MULTIPLICATION, SEM_DIVISION,
	SEM_WHILE_CYCLE, SEM_IF_STATEMENT, SEM_FUNCCALL, SEM_AND,
	SEM_OR, SEM_NOT, SEM_LT, SEM_EQ, SEM_GT, SEM_LTE, SEM_GTE, SEM_EMPTY};

#define VALUE_EMPTY 0

typedef struct AST {
	int semantic;	
	int value; /* Value for constants, index for ids */

	struct AST *child;
	struct AST *sibling;
	struct AST *parent;
} AST;

AST* CreateASTNode(int semantic, int value);
void AddASTChild(AST* parent, AST* child);
void DumpAST(AST *ast);

#endif /* AST_H */
