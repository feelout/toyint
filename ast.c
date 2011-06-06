#include <stdlib.h>
#include "ast.h"

AST* CreateASTNode(int semantic, int value) {
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
