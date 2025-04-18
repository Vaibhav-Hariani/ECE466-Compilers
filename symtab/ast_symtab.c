#include "ast_symtab.h"

#include <stdlib.h>

#include "symtab.tab.h"

// #define new_ast_node calloc(1, sizeof(struct ast_node))

ast_tab_t *new_ast_tab(ast_tab_t *parent, char scope_type) {
    return NULL;
}

int del_ast_tab(ast_tab_t *tab) {

}

ast_sym_t *lookup(ast_tab_t *tab, char *name, char sym_type) {
    return NULL;
}

int enter(ast_tab_t *tab, ast_sym_t *sym, char replace_dup) {
    return 0;
}

ast_sym_t *new_ast_sym(char *name, char stg_type, ast_type_t *type,
        char *filename, int line){
    return NULL;
}

ast_type_t *new_ast_type(char type, char qual, void *node){
    return NULL;
}

struct ast_scal *new_ast_scal(char scal_type){
    return NULL;
}

struct ast_ptr *new_ast_ptr(ast_type_t *to){
    return NULL;
}

struct ast_ary *new_ast_ary(int size, ast_type_t *elem){
    return NULL;
}

struct ast_func *new_ast_func(ast_type_t *ret, struct ast_proto *param_list){
    return NULL;
}

struct ast_stru *new_ast_stru(char is_complete, struct ast_smembs *smembs){
    return NULL;
}

struct ast_unio *new_ast_unio(char is_complete, struct ast_umembs *umembs){
    return NULL;
}

struct ast_enu *new_ast_enu(struct ast_enumcs *enumcs){
    return NULL;
}
