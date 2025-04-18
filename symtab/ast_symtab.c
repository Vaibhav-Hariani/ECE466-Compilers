#include "ast_symtab.h"

#include <stdlib.h>
#include <string.h>

#include "symtab.tab.h"

union ast_type *new_ast_scal(char unsign, char scal_type){
    struct ast_scal *scal;
    union ast_type *node;

    scal = calloc(1, sizeof(struct ast_scal));
    scal->unsign = unsign;
    scal->scal_type = scal_type;

    node = calloc(1, sizeof (union ast_type));
    node->scal = scal;
    return node;
}

union ast_type *new_ast_ptr(ast_data_t *to){
    struct ast_ptr *ptr;
    union ast_type *node;

    ptr = calloc(1, sizeof(struct ast_ptr));
    ptr->to = to;

    node = calloc(1, sizeof (union ast_type));
    node->ptr = ptr;
    return node;
}

union ast_type *new_ast_ary(TypedNumber size, ast_data_t *elem){
    struct ast_ary *ary;
    union ast_type *node;

    ary = calloc(1, sizeof(struct ast_ary));
    ary->elem = elem;
    if (size.type > TYPE_ULLI // floating point
    || (size.type % 2 == 0 && size.val.i < 0) // negative
    /*|| (unsigned long long) size.val.i >= some size limit?*/) {
        // ERROR invalid array size
    }
    ary->size = size.val.i;

    node = calloc(1, sizeof (union ast_type));
    node->ary = ary;
    return node;
}

union ast_type *new_ast_func(char is_complete, int is_inline,
        ast_data_t *ret, ast_tab_t *params){
    struct ast_func *func;
    union ast_type *node;

    func = calloc(1, sizeof(struct ast_func));
    func->is_complete = is_complete;
    func->is_inline = is_inline;
    func->ret = ret;
    func->params = params;

    node = calloc(1, sizeof (union ast_type));
    node->func = func;
    return node;
}

union ast_type *new_ast_param(ast_data_t *is){
    struct ast_param *param;
    union ast_type *node;

    param = calloc(1, sizeof(struct ast_param));
    param->is = is;

    node = calloc(1, sizeof (union ast_type));
    node->param = param;
    return node;
}

union ast_type *new_ast_stru(char is_complete, ast_tab_t *minitab){
    struct ast_stru *stru;
    union ast_type *node;

    stru = calloc(1, sizeof(struct ast_stru));
    stru->is_complete = is_complete;
    stru->minitab = minitab;

    node = calloc(1, sizeof (union ast_type));
    node->stru = stru;
    return node;
}

union ast_type *new_ast_unio(char is_complete, ast_tab_t *minitab){
    struct ast_unio *unio;
    union ast_type *node;

    unio = calloc(1, sizeof(struct ast_unio));
    unio->is_complete = is_complete;
    unio->minitab = minitab;

    node = calloc(1, sizeof (union ast_type));
    node->unio = unio;
    return node;
}

union ast_type *new_ast_enu(ast_tab_t *minitab){
    struct ast_enu *enu;
    union ast_type *node;
 
    enu = calloc(1, sizeof(struct ast_enu));
    enu->minitab = minitab;

    node = calloc(1, sizeof (union ast_type));
    node->enu = enu;
    return node;
}

union ast_type *new_ast_label(char is_complete){
    struct ast_label *label;
    union ast_type *node;
 
    label = calloc(1, sizeof(struct ast_label));
    label->is_complete = is_complete;

    node = calloc(1, sizeof (union ast_type));
    node->label = label;
    return node;
}

ast_data_t *new_ast_data(int size, char data_type, char qual, union ast_type *node){
    ast_data_t *data;

    data = calloc(1, sizeof(ast_data_t));
    data->size = size;
    data->data_type = data_type;
    data->qual = qual;
    
    switch (data_type) {
        case DATA_SCAL:
            data->node->scal = node->scal;
            break;
        case DATA_PTR:
            data->node->ptr = node->ptr;
            break;
        case DATA_ARY:
            data->node->ary = node->ary;
            break;
        case DATA_FUNC:
            data->node->func = node->func;
            break;
        case DATA_PARAM:
            data->node->param = node->param;
            break;
        case DATA_STRU:
            data->node->stru = node->stru;
            break;
        case DATA_UNIO:
            data->node->unio = node->unio;
            break;
        case DATA_ENU:
            data->node->enu = node->enu;
            break;
        case DATA_LABEL:
            data->node->label = node->label;
            break;
    }

    return data;
}

int del_ast_data(ast_data_t *data) {
    switch (data->data_type) {
        case DATA_PTR:
            del_ast_data(data->node->ptr->to);
            break;
        case DATA_ARY:
            del_ast_data(data->node->ary->elem);
            break;
        case DATA_PARAM:
            del_ast_data(data->node->param->is);
            break;
        case DATA_FUNC:
            del_ast_data(data->node->func->ret);
            del_ast_tab(data->node->func->params);
            break;
        case DATA_STRU:
            del_ast_tab(data->node->stru->minitab);
            break;
        case DATA_UNIO:
            del_ast_tab(data->node->unio->minitab);
            break;
        case DATA_ENU:
            del_ast_tab(data->node->enu->minitab);
            break;
        // DATA_SCAL, DATA_LABEL fall through
        // typedef case not implemented
    }

    free(data->node);
    free(data);
    return 0;
}

ast_sym_t *new_ast_sym(char *name, char stg_type, ast_data_t *data,
        char *filename, int line){
    ast_sym_t *sym;

    sym = calloc(1, sizeof(ast_sym_t));
    sym->name = name;
    sym->stg_type = stg_type;
    sym->data = data;
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
    del_ast_data(sym->data);
    
    free(sym);
    return next;
}

ast_tab_t *new_ast_tab(ast_tab_t *parent, char scope_type,
    char *filename, int line) {
    ast_tab_t *tab;

    tab = calloc(1, sizeof(ast_tab_t));
    tab->parent = parent;
    tab->scope_type = scope_type;
    tab->filename = filename;
    tab->line = line;
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
    
    free(tab->filename);
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
    ast_sym_t *old, *next;
    int namespace;

    next = sym->next;
    sym->next = NULL;
    old = lookup(tab, sym->name, sym->sym_type);
    if (old != NULL && old->tab == tab) {
        if (replace_dup) {
            // external refdefinitions? see h&s 4.2.5

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
            // ERROR symbol already defined, cant replace
            return 2;
        }
    } else {
        namespace = NS_MISC;
        if (sym->sym_type == SYM_LABEL){
            namespace = NS_LABEL;
        } else if (sym->sym_type >= SYM_STRU_M) {
            namespace = NS_MEMB;
        } else if (sym->sym_type >= SYM_STRU_T) {
            namespace = NS_TAG;
        }

        switch (namespace) {
            case NS_MISC:
                sym->next = tab->misc;
                tab->misc = sym;
                break;
            case NS_TAG:
                sym->next = tab->tag;
                tab->tag = sym;
                break;
            case NS_MEMB:
                sym->next = tab->memb;
                tab->memb = sym;
                break;
            case NS_LABEL:
                sym->next = tab->label;
                tab->label = sym;
                break;
        }
    }

    enter(tab, next, replace_dup);
    return 0;
}
