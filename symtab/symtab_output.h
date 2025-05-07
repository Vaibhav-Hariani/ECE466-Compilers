#ifndef SYMTAB_OUTPUT_H
#define SYMTAB_OUTPUT_H

#include "table.h"
#include "symbol.h"
#include "data.h"

typedef struct ast_tab ast_tab_t;
typedef struct ast_sym ast_sym_t;
typedef struct ast_data ast_data_t;

// Prints storage class and data type information of
// all parameters stored in params list.
int print_params(ast_sym_t *params, int num_tabs);

// Prints the data type information stored in data,
// including nested type information.
int print_data(ast_data_t *data, int num_tabs);

// Prints definition information, scope and data type of
// memb, a member of the struct/union (disambiguated by
// data_name) sym.  
int print_memb_decl(ast_sym_t *memb, ast_sym_t *sym, int num_tabs, char *data_name);

// Prints declaration, scope, storage class and data type
// information of vars and functions.
int print_sym_decl(ast_sym_t *sym, int num_tabs);

// Prints definition, scope and data type infortmation of
// structs/unions.
int print_obj_def(ast_sym_t *sym, int num_tabs);

#endif // SYMTAB_OUTPUT_H