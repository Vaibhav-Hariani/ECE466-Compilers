#ifndef SYMBOL_H
#define SYMBOL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "yylval.h"

enum stg_type {
    STG_NONE = 0,
    STG_EXTERN_IMP,
    STG_EXTERN_EXP,
    STG_STATIC,
    STG_REGISTER,
    STG_AUTO_LOC,
    STG_AUTO_PAR,
    STG_TYPEDEF,
    STG_NA
};

enum sym_type {
    SYM_NONE = 0,
    /*misc*/
    SYM_VAR,
    SYM_TYPEDEF,
    SYM_FUNC,
    SYM_ENU_C,
    SYM_PARAM,
    /*tag*/
    SYM_STRU_T,
    SYM_UNIO_T,
    SYM_ENU_T,
    /*member*/
    SYM_STRU_M,
    SYM_UNIO_M,
    /*label*/
    SYM_LABEL
};

enum sco_type {
    SCO_NONE = 0,
    SCO_FILE,
    SCO_FUNC,
    SCO_VUNC,
    SCO_BLOCK,
    SCO_PROTO,
    SCO_STRUNIO,
    SCO_ENU
};

// Each symbol contains a pointer to the
// table that contains the symbol, the
// next element of the table in the same
// namespace, the file name and line
// number at which the symbol was defined
// and the type info.
typedef struct ast_sym {
    ast_sym_t *prev;
    char *filename;
    int start;
    int end;

    char *name;
    char stg_type;
    char sym_type;
    char sco_type;
    char is_inline;
    int offset;
    ast_data_t *data;
    ast_data_t *tail;
} ast_sym_t;

// Creates ast_sym_t object with the specified name, storage
// class and data type and returns its address. Assumes name,
// data type and filename are dynamically allocated and uses
// their same addresses.
ast_sym_t *new_ast_sym(char *name, char stg_type, char sym_type,
    ast_data_t *data, char *filename, int start);

// Destroys symbol pointed to by sym and all storage it
// consumes. Returns a pointer to the previous symbol in the
// list.
ast_sym_t *del_ast_sym(ast_sym_t *sym);

// Returns the first symbol in a list of symbols.
ast_sym_t *list_start(ast_sym_t *sym);

// Destroys all symbols in the symbol list pointed to by sym
// and all storage they consume.
int del_sym_list(ast_sym_t *sym);

// Sets the size of a struct data type whose members are
// stored in the struct's symbol table. Pads the struct such
// that all members would meet their respective alignment
// requirements in a struct array. Handles pointers to
// structs of its own type, nested definitions. Sets offsets
// for all members.
int struct_fix(ast_data_t *data);

// Sets the size of a union data type whose members are
// stored in the union's symbol table. Pads the union such
// that all members would meet their respective alignment
// requirements in a union array. Handles pointers to
// unions of its own type, nested definitions.
int union_fix(ast_data_t *data);

// Installs tail at the reasonable spot in sym's tail, which
// represents the current tail. Returns tail on success.
// Sets data and tail if currently NULL in sym.
ast_data_t *install_tail(ast_sym_t *sym, ast_data_t *tail);

#endif // SYMBOL_H