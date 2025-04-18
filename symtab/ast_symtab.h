#ifndef AST_SYMTAB_H
#define AST_SYMTAB_H

#include "yylval.h"

enum scope_type {
    SCOPE_FILE = 0,
    SCOPE_FUNC,
    SCOPE_BLOCK,
    SCOPE_PROTO
};

// namespace divisions commented here
enum sym_type {
    /*misc*/
    SYM_VAR = 0,
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

enum ns_type {
    NS_MISC = 0,
    NS_TAG,
    NS_MEMB,
    NS_LABEL
};

enum type_type {
    TYPE_SCAL = 0,
    TYPE_VAR,
    TYPE_PTR,
    TYPE_ARY,
    TYPE_FUNC,
    TYPE_PARAM,
    TYPE_STRU,
    TYPE_UNIO,
    TYPE_ENU,
    TYPE_LABEL
};

enum stg_type {
    STG_EXTERN_IMP = 0,
    STG_EXTERN_EXP,
    STG_STATIC,
    STG_REGISTER,
    STG_AUTO_LOC,
    STG_AUTO_PAR,
    STG_NA
};

// get/set with bitwise operations
// don't ask me why i did it this way
enum qualifiers {
    QUAL_UNSIGNED = 1,  // 0001
    QUAL_CONST = 2,     // 0010
    QUAL_VOLATILE = 4,  // 0100
    QUAL_RESTRICT = 8   // 1000
};


typedef struct ast_tab ast_tab_t;
typedef struct ast_sym ast_sym_t;
typedef struct ast_type ast_type_t;

// Contains an ast for some type.
struct ast_type {
    char type;
    char qual;
    union {
        struct ast_scal *scal;
        struct ast_var *var;
        struct ast_ptr *ptr;
        struct ast_ary *ary;
        struct ast_func *func;
        struct ast_param *param;
        struct ast_stru *stru;
        struct ast_unio *unio;
        struct ast_enu *enu;
        struct ast_label *label;
    } *node;
};

// Each symbol table contains a pointer
// to the parent symbol table (i.e. the
// symbol table of immediate enclosing
// scope), the type of scope and the
// first element of each namespace.
struct ast_tab {
    ast_tab_t *parent;
    char scope_type;

    ast_sym_t *misc;
    ast_sym_t *tag;
    ast_sym_t *memb;
    ast_sym_t *label;
};


// Each symbol contains a pointer to the
// table that contains the symbol, the
// next element of the table in the same
// namespace, the file name and line
// number at which the symbol was defined
// and the type info.
typedef struct ast_sym {
    ast_tab_t *tab;
    ast_sym_t *next;
    char *filename;
    int line;

    char *name;
    char stg_type;
    char sym_type;
    int offset;
    ast_type_t *type;
} ast_sym_t;

struct ast_scal {
    char scal_type;
};

struct ast_var {
    ast_type_t *is;
};

struct ast_ptr {
    ast_type_t *to;
};

struct ast_ary {
    int size;
    ast_type_t *elem;
};

// note: special qualifier rules
struct ast_param {
    ast_type_t *is;
};

// note: scope is always file/global per gcc
// but not necessarily so per standard.
struct ast_func {
    ast_type_t *ret;
    ast_tab_t *params;
};

struct ast_stru {
    char is_complete;
    ast_tab_t *minitab;
};

struct ast_unio {
    char is_complete;
    ast_tab_t *minitab;
};

// enums may not get implemented
struct ast_enu {
    ast_tab_t *minitab;
};

// note: scope is always function
struct ast_label {
    int is_complete;
};

struct ast_scal *new_ast_scal(char scal_type);
struct ast_var *new_ast_var(ast_type_t *is);
struct ast_ptr *new_ast_ptr(ast_type_t *to);
struct ast_ary *new_ast_ary(int size, ast_type_t *elem);
struct ast_param *new_ast_param(ast_type_t *is);
struct ast_func *new_ast_func(ast_type_t *ret, ast_tab_t *params);
struct ast_stru *new_ast_stru(char is_complete, ast_tab_t *minitab);
struct ast_unio *new_ast_unio(char is_complete, ast_tab_t *minitab);
struct ast_enu *new_ast_enu(ast_tab_t *minitab);
struct ast_label *new_ast_label(int is_complete);

// Creates ast_type_t object with specified type, qualifiers
// and type node and returns its address.
ast_type_t *new_ast_type(char type, char qual, void *node);

// Frees type pointed to by type, also freeing nested types.
int del_ast_type(ast_type_t *type);

// Creates ast_sym_t object with the specified name, storage
// class and type and returns its address. Assumes name,
// type and filename are dynamically allocated and uses
// their same addresses.
ast_sym_t *new_ast_sym(char *name, char stg_type, ast_type_t *type,
    char *filename, int line);

// Destroys symbol table entry pointed to by sym and all
// storage it consumes. Returns a pointer to the next
// symbol in the table.
ast_sym_t *del_ast_sym(ast_sym_t *sym);

// Creates a new symbol table with scope type scope_type
// whose enclosing scope has the symbol table pointed to
// by parent.
ast_tab_t *new_ast_tab(ast_tab_t *parent, char scope_type);

// Destroys symbol table pointed to by tab and all storage
// it consumes. Returns a pointer to the symbol table for
// the immediately enclosing scope.
ast_tab_t *del_ast_tab(ast_tab_t *tab);

// Searches for symbol called name in the appropriate
// namespace for sym_type in symbol table pointed to by
// tab and all enclosing scope symbol tables (in order of
// proximity), returning a pointer to the symbol table
// entry if it exists, NULL otherwise.
ast_sym_t *lookup(ast_tab_t *tab, char *name, char sym_type);

// Attempts to enter the symbol pointed to by sym into the
// table pointed to by tab. If replace_dup is 0, an error
// occurs if a symbol with the same name and namespace
// exists; otherwise, such a symbol is replaced. Returns 0
// on entry, 1 on replacement, 2 on error. On replacement,
// sym will change to point to the symbol in the tab.
int enter(ast_tab_t *tab, ast_sym_t *sym, char replace_dup);

#endif // AST_SYMTAB_H