#ifndef STMT_OUT_H
#define STMT_OUT_H

#include <stdio.h>

#include "expr_out.h"
#include "../util/stmt.h"

typedef struct ast_node ast_node;
typedef struct ast_sym ast_sym_t;
typedef struct ast_cpst ast_cpst_t;
typedef struct ast_stmt ast_stmt_t;

void print_stmt(ast_stmt_t *stmt);

void print_stmt_list(ast_stmt_t *stmt);

#endif // STMT_OUT_H