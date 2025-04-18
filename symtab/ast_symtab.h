#ifndef AST_SYMTAB_H
#define AST_SYMTAB_H

#include "yylval.h"

enum scope_type {
    SCOPE_FILE = 0,
    SCOPE_FUNC,
    SCOPE_BLOCK,
    SCOPE_PROTO,
    SCOPE_STRU,
    SCOPE_UNIO,
    SCOPE_ENU
};

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

enum data_type {
    DATA_SCAL = 0,
    DATA_PTR,
    DATA_ARY,
    DATA_FUNC,
    DATA_PARAM,
    DATA_STRU,
    DATA_UNIO,
    DATA_ENU,
    DATA_LABEL
};

enum scal_type {
    SCAL_SHORT=0,
    SCAL_INT,
    SCAL_LONG,
    SCAL_LONGLONG,
    SCAL_FLOAT,
    SCAL_DOUB,
    SCAL_LONGDOUB,
    SCAL_CHAR,
    SCAL_VOID
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
enum qual_type {
    QUAL_CONST = 1,     // 001
    QUAL_VOLATILE = 2,  // 010
    QUAL_RESTRICT = 3  // 100
};


typedef struct ast_tab ast_tab_t;
typedef struct ast_sym ast_sym_t;
typedef struct ast_data ast_data_t;

// Contains an ast for some data type.
struct ast_data {
    int size;
    char data_type;
    char qual;
    union ast_type {
        struct ast_scal *scal;
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
    char *filename;
    int line;

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
struct ast_sym {
    ast_tab_t *tab;
    ast_sym_t *next;
    char *filename;
    int line;

    char *name;
    char stg_type;
    char sym_type;
    int offset;
    ast_data_t *data;
};

struct ast_scal {
    char unsign;
    char scal_type;
};

struct ast_var {
    ast_data_t *is;
};

struct ast_ptr {
    ast_data_t *to;
};

struct ast_ary {
    int size;
    ast_data_t *elem;
};

// note: special qualifier rules
struct ast_param {
    ast_data_t *is;
};

// note: scope is always file/global per gcc
// but not necessarily so per standard.
struct ast_func {
    char is_complete;
    char is_inline;
    ast_data_t *ret;
    ast_tab_t *params;
};

struct ast_stru {
    char is_complete;
    ast_sym_t *sym;
    ast_tab_t *minitab;
};

struct ast_unio {
    char is_complete;
    ast_sym_t *sym;
    ast_tab_t *minitab;
};

// enums may not get implemented
struct ast_enu {
    ast_sym_t *sym;
    ast_tab_t *minitab;
};

// note: scope is always function
struct ast_label {
    char is_complete;
};

struct ast_scal *new_ast_scal(char unsign, char scal_type);
struct ast_var *new_ast_var(ast_data_t *is);
struct ast_ptr *new_ast_ptr(ast_data_t *to);
struct ast_ary *new_ast_ary(TypedNumber size, ast_data_t *elem);
struct ast_param *new_ast_param(ast_data_t *is);
struct ast_func *new_ast_func(char is_complete, int is_inline,
    ast_data_t *ret, ast_tab_t *params);
struct ast_stru *new_ast_stru(char is_complete, ast_tab_t *minitab);
struct ast_unio *new_ast_unio(char is_complete, ast_tab_t *minitab);
struct ast_enu *new_ast_enu(ast_tab_t *minitab);
struct ast_label *new_ast_label(char is_complete);

// Creates ast_data_t object with specified type, qualifiers
// and type node and returns its address.
ast_data_t *new_ast_data(int size, char data_type, char qual, union ast_type *node);

// Recursively frees data type pointed to by data.
int del_ast_data(ast_data_t *data);

// Creates ast_sym_t object with the specified name, storage
// class and data type and returns its address. Assumes name,
// data type and filename are dynamically allocated and uses
// their same addresses.
ast_sym_t *new_ast_sym(char *name, char stg_type, ast_data_t *data,
    char *filename, int line);

// Destroys symbol table entry pointed to by sym and all
// storage it consumes. Returns a pointer to the next
// symbol in the table.
ast_sym_t *del_ast_sym(ast_sym_t *sym);

// Creates a new symbol table with scope type scope_type
// whose enclosing scope has the symbol table pointed to
// by parent. Assumes parent and filename are dynamically
// allocated and uses their addresses.
ast_tab_t *new_ast_tab(ast_tab_t *parent, char scope_type,
    char *filename, int line);

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

// Attempts to enter the symbol list pointed to by sym and
// the chain of next ast_sym_t *s into the table pointed to
// by tab. If replace_dup is 0, an error occurs if a symbol
// with the same name and namespace exists; otherwise, such a
// symbol is replaced. Returns 0 on entry, 1 on replacement,
// 2 on error. On replacement, sym will change to point to
// the symbol in the tab.
int enter(ast_tab_t *tab, ast_sym_t *sym, char replace_dup);

#endif // AST_SYMTAB_H