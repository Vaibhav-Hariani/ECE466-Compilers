#ifndef EXPR_OUT_H
#define EXPR_OUT_H

#include "../util/expr.h"

typedef struct ast_node ast_node;

void print_spaces(int num_spaces);

ast_node* print_ast(ast_node* expr, int num_spaces);

void print_recurse(ast_node* expr, int num_spaces, char** tokens);

#endif // EXPR_OUTPUT_H