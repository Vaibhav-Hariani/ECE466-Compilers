#include "ast_symtab.h"

#include <stdlib.h>
#include <string.h>

#include "symtab.tab.h"

struct ast_scal *new_ast_scal(char scal_type){
    struct ast_scal *scal;

    scal = calloc(1, sizeof(struct ast_scal));
    scal->scal_type = scal_type;
    return scal;
}

struct ast_var *new_ast_var(ast_type_t *is){
    struct ast_var *var;

    var = calloc(1, sizeof(struct ast_var));
    var->is = is;
    return var;
}

struct ast_ptr *new_ast_ptr(ast_type_t *to){
    struct ast_ptr *ptr;

    ptr = calloc(1, sizeof(struct ast_ptr));
    ptr->to = to;
    return ptr;
}

struct ast_ary *new_ast_ary(int size, ast_type_t *elem){
    struct ast_ary *ary;

    ary = calloc(1, sizeof(struct ast_ary));
    ary->elem = elem;
    ary->size = size;
    return ary;
}

struct ast_func *new_ast_func(ast_type_t *ret, ast_tab_t *params){
    struct ast_func *func;

    func = calloc(1, sizeof(struct ast_func));
    func->ret = ret;
    func->params = params;
    return func;
}

struct ast_param *new_ast_param(ast_type_t *is){
    struct ast_param *param;

    param = calloc(1, sizeof(struct ast_param));
    param->is = is;
    return param;
}

struct ast_stru *new_ast_stru(char is_complete, ast_tab_t *minitab){
    struct ast_stru *stru;

    stru = calloc(1, sizeof(struct ast_stru));
    stru->is_complete = is_complete;
    stru->minitab = minitab;
    return stru;
}

struct ast_unio *new_ast_unio(char is_complete, ast_tab_t *minitab){
    struct ast_unio *unio;

    unio = calloc(1, sizeof(struct ast_unio));
    unio->is_complete = is_complete;
    unio->minitab = minitab;
    return unio;
}

struct ast_enu *new_ast_enu(ast_tab_t *minitab){
    struct ast_enu *enu;

    enu = calloc(1, sizeof(struct ast_enu));
    enu->minitab = minitab;
    return enu;
}

ast_type_t *new_ast_type(char type, char qual, void *node){
    ast_type_t *symtype;

    symtype = calloc(1, sizeof(ast_type_t));
    symtype->type = type;
    symtype->qual = qual;
    
    switch (type) {
        case TYPE_SCAL:
            symtype->node->scal = (struct ast_scal *) node;
            break;
        case TYPE_VAR:
            symtype->node->var = (struct ast_var *) node;
            break;
        case TYPE_PTR:
            symtype->node->ptr = (struct ast_ptr *) node;
            break;
        case TYPE_ARY:
            symtype->node->ary = (struct ast_ary *) node;
            break;
        case TYPE_FUNC:
            symtype->node->func = (struct ast_func *) node;
            break;
        case TYPE_PARAM:
            symtype->node->param = (struct ast_param *) node;
            break;
        case TYPE_STRU:
            symtype->node->stru = (struct ast_stru *) node;
            break;
        case TYPE_UNIO:
            symtype->node->unio = (struct ast_unio *) node;
            break;
        case TYPE_ENU:
            symtype->node->enu = (struct ast_enu *) node;
            break;
        case TYPE_LABEL:
            symtype->node->label = (struct ast_label *) node;
            break;
    }

    return symtype;
}

int del_ast_type(ast_type_t *type) {
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
            del_ast_tab(type->node->func->params);
            break;
        case TYPE_STRU:
            del_ast_tab(type->node->stru->minitab);
            break;
        case TYPE_UNIO:
            del_ast_tab(type->node->unio->minitab);
            break;
        case TYPE_ENU:
            del_ast_tab(type->node->enu->minitab);
            break;
        // TYPE_SCAL, TYPE_LABEL fall through
        // typedef case not implemented
    }

    free(type->node);
    free(type);
    return 0;
}

ast_sym_t *new_ast_sym(char *name, char stg_type, ast_type_t *type,
        char *filename, int line){
    ast_sym_t *sym;

    sym = calloc(1, sizeof(ast_sym_t));
    sym->name = name;
    sym->stg_type = stg_type;
    sym->filename = filename;
    sym->line = line;
    return sym;
}

ast_sym_t *del_ast_sym(ast_sym_t *sym) {
    ast_sym_t *next;

    next  = sym->next;
    free(sym->name);
    free(sym->filename);
    free(sym->tab);
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

ast_tab_t *del_ast_tab(ast_tab_t *tab) {
    ast_sym_t *next;
    ast_tab_t *encl;

    encl = tab->parent;

    while (tab->misc != NULL) {
        next = tab->misc->next;
        del_ast_sym(tab->misc);
        tab->misc = next;
    }

    while (tab->tag != NULL) {
        next = tab->tag->next;
        del_ast_sym(tab->tag);
        tab->tag = next;
    }

    while (tab->memb != NULL) {
        next = tab->memb->next;
        del_ast_sym(tab->memb);
        tab->memb = next;
    }

    while (tab->label != NULL) {
        next = tab->label->next;
        del_ast_sym(tab->label);
        tab->label = next;
    }
    
    return encl;
}

ast_sym_t *lookup(ast_tab_t *tab, char *name, char sym_type) {
    int namespace;
    ast_sym_t *sym;
    ast_tab_t *curr;

    namespace = NS_MISC;
    if (sym_type == SYM_LABEL){
        namespace = NS_LABEL;
    } else if (sym_type >= SYM_STRU_M) {
        namespace = NS_MEMB;
    } else if (sym_type >= SYM_STRU_T) {
        namespace = NS_TAG;
    }

    curr = tab;
    while (curr != NULL) {
        switch (namespace) {
            case NS_MISC:
                sym = curr->misc;
                break;
            case NS_TAG:
                sym = curr->tag;
                break;
            case NS_MEMB:
                sym = curr->memb;
                break;
            case NS_LABEL:
                // labels only in function scope
                if (curr->scope_type != SCOPE_FUNC) {
                    curr = curr->parent;
                    continue;
                }
                sym = curr->label;
                break;
        }

        while (sym != NULL) {
            if (!strcmp(sym->name, name)) {
                return sym;
            }
            sym = sym->next;
        }
        curr = curr->parent;
    }
    return NULL;
}

int enter(ast_tab_t *tab, ast_sym_t *sym, char replace_dup) {
    ast_sym_t *old;

    old = lookup(tab, sym->name, sym->sym_type);
    if (old != NULL && old->tab == tab) {
        if (replace_dup) {
            // potentially have to check if types are compatible.
            // if compatible, we might have to construct
            // a composite type, which we can do in old by combing
            // through old and sym and comparing type attributes.
            // we do this in old because there's already some symbol
            // in the table pointing to old as the next symbol.

            // once all of that is done, we have redeclared old
            free(old->filename);
            old->filename = strdup(sym->filename);
            old->line = sym->line;
            del_ast_sym(sym);
            sym = old;
            return 1;
        } else {
            // ERROR
        }
    }
    return 0;
}
