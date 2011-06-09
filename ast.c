#include <stdio.h>
#include <stdlib.h>
#include "ast.h"

char* semanticToString[] = {
	":=", "id", "const", "+", "-", "*", "/",
	"while", "if", "call", "and", "or", "not",
	"<", "=", ">", "<=", ">=", "print", "read", "<>"
};

AST* CreateASTNode(int semantic, Value* value) {
	AST* node = (AST*)malloc(sizeof(AST));

	node->semantic = semantic;
	node->value = value;
	
	node->child = NULL;
	node->sibling = NULL;
	node->parent = NULL;

	return node;
}

void AddASTChild(AST* parent, AST* child) {
	AST *curChild = parent->child;

	child->parent = parent;

	if(!curChild) {
		parent->child = child;
		return;
	}
	
	while(curChild->sibling) {
		curChild = curChild->sibling;
	}

	curChild->sibling = child;
}

void DumpASTWithIndent(AST *ast, unsigned int indent) {
	unsigned int i;
	AST *node;

	for(node = ast; node; node = node->sibling) {
		for(i = 0; i < indent; ++i) {
			printf("\t");
		}
		printf("%s", semanticToString[node->semantic]);
		if(node->semantic == SEM_ID || node->semantic == SEM_CONSTANT) {
			char* value_repr = ValueToString(node->value);
			printf("(%s)", value_repr);
			free(value_repr);
		}
		printf("\n");

		if(node->child) {
			DumpASTWithIndent(node->child, indent+1);
		}
	}
}

void DumpAST(AST *ast) {
	DumpASTWithIndent(ast, 0);	
}
