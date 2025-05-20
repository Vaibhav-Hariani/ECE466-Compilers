#ifndef SYMBOL_H
#define SYMBOL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "data.h"
#include "table.h"

typedef struct ast_data ast_data_t;
typedef struct ast_tab ast_tab_t;

enum stg_type {
    STG_NONE = 0,
    STG_EXTERN_IMP = 1,
    STG_EXTERN_EXP = 2,
    STG_STATIC = 4,
    STG_REGISTER = 8,
    STG_AUTO_LOC = 16,
    STG_AUTO_PAR = 32,
    STG_NA = 64,
    STG_TYPEDEF = 128
};

enum sym_type {
    SYM_NONE = 0,
    /*misc*/
    SYM_VAR = 1,
    SYM_TYPEDEF = 2,
    SYM_FUNC = 4,
    SYM_ENU_C = 8,
    SYM_PARAM = 16,
    /*tag*/
    SYM_STRU_T = 32,
    SYM_UNIO_T = 64,
    SYM_ENU_T = 128,
    /*member*/
    SYM_STRU_M = 256,
    SYM_UNIO_M = 512,
    /*label*/
    SYM_LABEL = 1024
};

enum sco_type {
    SCO_NONE = 0,
    SCO_FILE = 1,
    SCO_FUNC = 2,
    SCO_BLOCK = 4,
    SCO_PROTO = 8,
    SCO_STRUNIO = 16,
    SCO_ENU = 32
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
    short sym_type;
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
ast_sym_t *new_ast_sym(char *name, char stg_type, short sym_type,
    ast_data_t *data, char *filename, int start);

// Returns a deep copy of sym and all of its data type
// information. The copy's prev field is NULL.
ast_sym_t *copy_ast_sym(ast_sym_t *sym);

// Makes a deep copy of the symbol list to whose final
// member sym points.
ast_sym_t *copy_sym_list(ast_sym_t *sym);

// Destroys symbol pointed to by sym and all storage it
// consumes. Returns a pointer to the previous symbol in the
// list.
ast_sym_t *del_ast_sym(ast_sym_t *sym);

// Returns the first symbol in a list of symbols.
ast_sym_t *list_start(ast_sym_t *sym);

// Destroys all symbols in the symbol list pointed to by sym
// and all storage they consume.
int del_sym_list(ast_sym_t *sym);

// When passed a symbol whose tail data type is sue (struct/
// union/enum placeholder), attempts to find the respective
// struct/union/enum with the same tag name in the closest
// scope and replaces the tail with that type. Returns NULL
// if no suitable struct/union/enum type is found.
ast_sym_t *resolve_tag(ast_tab_t *tab, ast_sym_t *sym);

// Sets the size of a struct data type whose members are
// stored in the struct's symbol table. Pads the struct such
// that all members would meet their respective alignment
// requirements in a struct array. Handles pointers to
// structs of its own type, nested definitions. Sets offsets
// for all members.
int struct_fix(ast_sym_t *tag, ast_data_t *data);

// Sets the size of a union data type whose members are
// stored in the union's symbol table. Pads the union such
// that all members would meet their respective alignment
// requirements in a union array. Handles pointers to
// unions of its own type, nested definitions.
int union_fix(ast_sym_t *tag, ast_data_t *data);

// Finds the parent type of tail in the multi-layer data
// type data, returning NULL if no parent type found. 
ast_data_t *get_tail_head(ast_data_t *data, ast_data_t *tail);

// Finds the tail of the (potentially) multi-layer data type
// data. 
ast_data_t *get_tail(ast_data_t *data);

// Installs tail at the reasonable spot in sym's tail, which
// represents the current tail. Returns tail on success.
// Sets data and tail if currently NULL in sym.
ast_data_t *install_tail(ast_sym_t *sym, ast_data_t *tail);

#endif // SYMBOL_H