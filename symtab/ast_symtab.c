#include "ast_symtab.h"

#include <stdlib.h>

#include "symtab.tab.h"

struct ast_scal *new_ast_scal(char scal_type){
    return NULL;
}

struct ast_var *new_ast_var(ast_type_t *is){
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

struct ast_param *new_ast_param(ast_type_t *is){
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

ast_type_t *new_ast_type(char type, char qual, void *node){
    return NULL;
}

int del_ast_type(ast_type_t *type) {
    struct ast_proto *param, *next;
    struct ast_smembs *scurr, *snext;
    struct ast_umembs *ucurr, *unext;
    struct ast_enumcs *ecurr, *enext;

    // typedef case omitted
    switch (type->type) {
        
        case TYPE_VAR:
            del_ast_type(type->node->var->is);
            break;
        
        case TYPE_PTR:
            del_ast_type(type->node->ptr->to);
            break;
        
        case TYPE_ARY:
            del_ast_type(type->node->ary->elem);
            break;
        
        case TYPE_PARAM:
            del_ast_type(type->node->param->is);
            break;
        
        case TYPE_FUNC:
            del_ast_type(type->node->func->ret);
            param = type->node->func->param_list;
            while (param != NULL) {
                next = param->next;
                del_ast_type(param->param);
                free(param->name);
                free(param);
                param = next;
            }
            break;
        
        case TYPE_STRU:
            scurr = type->node->stru->smembs;
            while (scurr != NULL) {
                snext = scurr->next;
                del_ast_type(scurr->type);
                free(scurr->name);
                free(scurr);
                scurr = snext;
            }
            break;
        
        case TYPE_UNIO:
            ucurr = type->node->unio->umembs;
            while (ucurr != NULL) {
                unext = ucurr->next;
                del_ast_type(ucurr->type);
                free(ucurr->name);
                free(ucurr);
                ucurr = unext;
            }
            break;
        
        case TYPE_ENU:
            ecurr = type->node->enu->enumcs;
            while (ecurr != NULL) {
                enext = ecurr->next;
                free(ecurr->name);
                free(ecurr);
                ecurr = enext;
            }
            break;
    }

    free(type->node);
    free(type);
    return 0;
}

ast_sym_t *new_ast_sym(char *name, char stg_type, ast_type_t *type,
        char *filename, int line){
    return NULL;
}

ast_sym_t *del_ast_sym(ast_sym_t *sym) {
    ast_sym_t *next;

    next = sym->next;
    free(sym->name);
    free(sym->filename);
    free(sym->table);
    del_ast_type(sym->type);
    
    free(sym);
    return next;
}

ast_tab_t *new_ast_tab(ast_tab_t *parent, char scope_type) {
    ast_tab_t *tab;

    tab = calloc(1, sizeof(ast_tab_t));
    tab->parent = parent;
    tab->scope_type = scope_type;
    return tab;
}

int del_ast_tab(ast_tab_t *tab, ast_tab_t *curr) {
    ast_sym_t *next, *temp;

    curr = tab->parent;

    temp = tab->misc;
    while (temp != NULL) {
        next = temp->next;
        del_ast_sym(temp);
        temp = next;
    }

    temp = tab->tags;
    while (temp != NULL) {
        next = temp->next;
        del_ast_sym(temp);
        temp = next;
    }

    temp = tab->labels;
    while (temp != NULL) {
        next = temp->next;
        del_ast_sym(temp);
        temp = next;
    }
    
    return 0;
}

ast_sym_t *lookup(ast_tab_t *tab, char *name, char sym_type) {
    return NULL;
}

int enter(ast_tab_t *tab, ast_sym_t *sym, char replace_dup) {
    return 0;
}
