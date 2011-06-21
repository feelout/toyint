#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "ast.h"
#include "scope.h"
#include "idtable.h"

void InterpretAST(AST* ast, Scope* scope, IDTable* id_table);

#endif // INTERPRETER_H
