#ifndef AST_NODES_H
#define AST_NODES_H

#include "yylval.h"

enum node_type {
    AST_binop=0,
    AST_ternop,
    AST_unop,
    AST_varlen,
    AST_lvalue,
    AST_assign,
    AST_special,
    AST_ident,
    AST_string,
    AST_charlit,
    AST_num,
    //Not used particularly, though it's valid for function calls and array definition
    AST_EMPTY,
};

typedef struct ast_node ast_node_t;

struct binop {
    ast_node_t* expr_1;
    ast_node_t* expr_2;
    int opcode;
};

struct ternop {
    ast_node_t* expr_1;
    ast_node_t* expr_2;
    ast_node_t* expr_3;
};

struct lvalue {
    ast_node_t* expr;
};


struct assign {
    ast_node_t* lvalue;
    ast_node_t* rvalue;
    int opcode;
};

//Special struct for function calls
struct special {
    ast_node_t* expr_1;
    ast_node_t* expr_2;
    int opcode;
};


struct unop {
    ast_node_t* expr;
    int opcode;
    int sequence;
};

struct ast_node {
    int is_lval;
    //Reference to what obj is
    int type;
    //the object itself
    union {
        struct binop* b;
        struct ternop* t;
        struct unop* u;
        struct assign* a;
        struct special* s;
        //Non-recursive types can just live here
        char charlit;
        TypedNumber num;
        SizedString str;
        char* ident;
    } obj;
};

ast_node_t* new_ast_ident(char* c);
ast_node_t* new_ast_num(TypedNumber n);
ast_node_t* new_ast_charlit(char c);

ast_node_t* new_ast_string(SizedString s);

ast_node_t* new_ast_binop(int type, ast_node_t* expr1, ast_node_t* expr2, int op);

//Just sets a flag if desired
ast_node_t* new_ast_lvalue(ast_node_t* expr);


ast_node_t* new_ast_ternop(int type, ast_node_t* expr1, ast_node_t* expr2, ast_node_t* expr3);

ast_node_t* new_ast_unop(ast_node_t* expr, int op, int dir);


#endif