#ifndef EXPR_OUT_H
#define EXPR_OUT_H

#include "../util/expr.h"

typedef struct ast_node ast_node;

//Only function that needs to be exposed
ast_node* print_ast(ast_node* expr);

void print_recurse(ast_node* expr, int num_tabs, char** tokens);

#endif // EXPR_OUTPUT_H