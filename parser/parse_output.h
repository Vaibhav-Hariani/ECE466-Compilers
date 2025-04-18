
#ifndef PARSE_OUTPUT_H
#define PARSE_OUTPUT_H

#include "ast_nodes.h"

//Only function that needs to be exposed
ast_node_t* print_ast(ast_node_t* expr);

void print_recurse(ast_node_t* expr, int num_tabs, char** tokens);

#endif