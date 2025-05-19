#ifndef EXPR_H
#define EXPR_H

#include "yylval.h"

enum node_type {
    AST_binop=0,
    AST_ternop,
    AST_unop,
    AST_assign,
    AST_list,
    AST_funct,
    AST_ident,
    AST_string,
    AST_charlit,
    AST_num,
};

struct ast_node typedef ast_node;


struct list_node {
    struct ast_node* cur;
    struct list_node* next;
};

struct binop {
    ast_node* expr_1;
    ast_node* expr_2;
    int opcode;
};

struct ternop {
    ast_node* expr_1;
    ast_node* expr_2;
    ast_node* expr_3;
};

struct lvalue {
    ast_node* expr;
};


struct assign {
    ast_node* lvalue;
    ast_node* rvalue;
    int opcode;
};

//Special struct for function calls
struct funct {
    ast_node* name;
    struct list_node* args;
};


struct unop {
    ast_node* expr;
    int opcode;
    int sequence;
};

typedef union ast_node_t {
    struct binop* b;
    struct ternop* t;
    struct unop* u;
    struct assign* a;
    struct funct* f;
    struct list_node* l;
    //Non-recursive types can just live here
    char charlit;
    TypedNumber num;
    SizedString str;
    char* ident;
} ast_node_t;


struct ast_node { 
    int is_lval;
    //Reference to what obj is
    int type;
    //the object itself
    union ast_node_t obj;   
    //Because for some reason this is necessary for symtab lookups
    int line_num;
} typedef ast_node;



ast_node* new_ast_ident(char* c);
ast_node* new_ast_num(TypedNumber n);
ast_node* new_ast_charlit(char c);

ast_node* new_ast_string(SizedString s);

ast_node* new_ast_double(int type, ast_node* expr1, ast_node* expr2, int op);

// //Just sets a flag if desired
// ast_node* new_ast_lvalue(ast_node* expr);

ast_node* ast_array_exp(ast_node* expr1, ast_node* expr2);

ast_node* new_ast_list(ast_node* head);
ast_node* append_ast_list(ast_node* tail, ast_node* new);


ast_node* new_ast_ternop(int type, ast_node* expr1, ast_node* expr2, ast_node* expr3);

ast_node* new_ast_single(ast_node* expr, int op, int dir);

#endif // EXPR_H