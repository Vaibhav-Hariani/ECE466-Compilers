#ifndef DATA_H
#define DATA_H

#include "symbol.h"
#include "symtab.tab.h"

typedef struct ast_sym ast_sym_t;

enum data_type {
    DATA_NONE = 0,
    DATA_SCAL,
    DATA_PTR,
    DATA_ARY,
    DATA_FUNC,
    DATA_PARAM,
    DATA_SUE,
    DATA_STRU,
    DATA_UNIO,
    DATA_ENU,
    DATA_LABEL
};

// bitwise operation friendly
enum qual_type {
    QUAL_NONE = 0,
    QUAL_CONST = 1,     // 001
    QUAL_VOLATILE = 2,  // 010
    QUAL_RESTRICT = 4   // 100
};

enum scal_type {
    SCAL_NONE=0,
    SCAL_SHORT,
    SCAL_INT,
    SCAL_LONG,
    SCAL_LONGLONG,
    SCAL_FLOAT,
    SCAL_DOUB,
    SCAL_LONGDOUB,
    SCAL_CHAR,
    SCAL_VOID,
    SCAL_BOOL
};

// types of data type structs
union ast_type {
    struct ast_scal *scal;
    struct ast_ptr *ptr;
    struct ast_ary *ary;
    struct ast_func *func;
    struct ast_param *param;
    struct ast_sue *sue;
    struct ast_stru *stru;
    struct ast_unio *unio;
    struct ast_enu *enu;
    struct ast_label *label;
};

// Contains an ast for some data type.
typedef struct ast_data {
    int size;
    char data_type;
    char qual;
    union ast_type *node;
} ast_data_t;

struct ast_scal {
    char unsign;
    char scal_type;
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
    char is_variadic;
    ast_data_t *ret;
    ast_sym_t *params;
};

struct ast_sue {
    char data_type;
    char *name;
};

struct ast_stru {
    char is_complete;
    ast_sym_t *tag;
    ast_sym_t *membs;
};

struct ast_unio {
    char is_complete;
    ast_sym_t *tag;
    ast_sym_t *membs;
};

struct ast_enu {
    char is_complete;
    ast_sym_t *tag;
};

// note: scope is always function
struct ast_label {
    char is_complete;
};

union ast_type *new_ast_scal(char unsign, char scal_type);
union ast_type *new_ast_ptr(ast_data_t *to);
union ast_type *new_ast_ary(TypedNumber size, ast_data_t *elem);
union ast_type *new_ast_param(ast_data_t *is);
union ast_type *new_ast_func(char is_complete, char is_variadic, ast_data_t *ret, ast_sym_t *params);
union ast_type *new_ast_sue(char data_type, char *name);
union ast_type *new_ast_stru(char is_complete, ast_sym_t *tag, ast_sym_t *memb);
union ast_type *new_ast_unio(char is_complete, ast_sym_t *tag, ast_sym_t *memb);
union ast_type *new_ast_enu(char is_complete, ast_sym_t *tag);
union ast_type *new_ast_label(char is_complete);

// Creates ast_data_t object with specified type, qualifiers
// and type node and returns its address.
ast_data_t *new_ast_data(int size, char data_type, char qual, union ast_type *node);

// Recursively creates a deep copy of data down to the
// specified depth, beyond which further nested data is
// shallow copied. Depth = -1 results in a full deep copy
// aside from struct/union/enum tags, which are always
// shallow copied to preserve type compatibility.
ast_data_t *copy_ast_data(ast_data_t *data, int depth);

// Dynamically allocates and creates a pointer to the
// composite type for src and target if both are compatible.
// Returns NULL if no such type could be created.
ast_data_t *comb_ast_data(ast_data_t *src, ast_data_t *target);

// Recursively frees data type pointed to by data.
int del_ast_data(ast_data_t *data);

#endif // DATA_H