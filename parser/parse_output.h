#ifndef PARSE_OUTPUT_H
#define PARSE_OUTPUT_H

#include "ast_nodes.h"

//Only function that needs to be exposed
ast_node* print_ast(ast_node* expr);

void print_recurse(ast_node* expr, int num_tabs, char** tokens);

#endif // PARSE_OUTPUT_H