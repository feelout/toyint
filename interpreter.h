#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "ast.h"
#include "scope.h"

void InterpretAST(AST* ast, Scope* scope);

#endif // INTERPRETER_H
