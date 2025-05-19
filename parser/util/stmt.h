#ifndef STMT_H
#define STMT_H

#include "yylval.h"
#include "expr.h"
#include "symbol.h"
#include "../parser.tab.h"

enum stmt_type {
    STMT_EXPR = 0,
    STMT_BLOCK,
    STMT_IF,
    STMT_FOR,
    STMT_WHILE,
    STMT_DOWHILE,
    STMT_SWITCH
};

typedef struct ast_sym ast_sym_t;
typedef struct ast_node ast_node;
typedef struct ast_cpst ast_cpst_t;
typedef struct ast_stmt ast_stmt_t;

struct ast_if {
    ast_node *cond;
    ast_cpst_t *icpst;
    ast_cpst_t *ecpst;
};

struct ast_for {
    ast_node *init;
    ast_node *cond;
    ast_node *adv;
    ast_cpst_t *cpst;
};

struct ast_while {
    ast_node *cond;
    ast_cpst_t *cpst;
};

struct ast_dowhile {
    ast_node *cond;
    ast_cpst_t *cpst;
};

struct ast_switch {
    /*not implemented*/
};

union ast_st {
    ast_node *expr;
    ast_cpst_t *block;
    struct ast_if *ifst;
    struct ast_for *forst;
    struct ast_while *whilst;
    struct ast_dowhile *dowhilst;
};

typedef struct ast_stmt {
    ast_stmt_t *next;
    union ast_st *st;
    char stmt_type;
} ast_stmt_t;

typedef struct ast_cpst {
    ast_stmt_t *stmt;
    ast_sym_t *sym;
} ast_cpst_t;

union ast_st *new_ast_expr(ast_node *expr);
union ast_st *new_ast_block(ast_cpst_t *cpst);
union ast_st *new_ast_if(ast_node *cond, ast_cpst_t *icpst, ast_cpst_t *ecpst);
union ast_st *new_ast_for(ast_node *init, ast_node *cond, ast_node *adv, ast_cpst_t *cpst);
union ast_st *new_ast_while(ast_node *cond, ast_cpst_t *cpst);
union ast_st *new_ast_dowhile(ast_node *cond, ast_cpst_t *cpst);

ast_stmt_t *new_ast_stmt(union ast_st *st, char stmt_type, ast_stmt_t *next);

ast_cpst_t *new_ast_cpst(ast_stmt_t *stmt, ast_sym_t *sym);

#endif // STMT_H