#ifndef SYMTAB_OUTPUT_H
#define SYMTAB_OUTPUT_H

#include "ast_symtab.h"

// Prints storage class and data type information of
// all parameters stored in the misc namespace of tab.
int print_params(ast_tab_t *tab, int num_tabs);

// Prints the data type information stored in data,
// including nested type information.
int print_data(ast_data_t *data, int num_tabs);

// Prints definition information, scope and data type of
// memb, a member of the struct/union/enum (disambiguated
// by data_name) sym.  
int print_memb_decl(ast_sym_t *memb, ast_sym_t *sym, int num_tabs, char *data_name);

// Prints declaration, scope, storage class and data type
// information of vars and functions.
int print_sym_decl(ast_sym_t *sym, int num_tabs);

// Prints definition, scope and data type infortmation of
// structs/unions/enums.
int print_obj_def(ast_sym_t *sym, int num_tabs);

#endif // SYMTAB_OUTPUT_H