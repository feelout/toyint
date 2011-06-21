#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "idtable.h"

AST* ParseFile(char* filename, IDTable* id_table);

#endif /* PARSER_H */
