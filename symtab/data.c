#include "data.h"

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

union ast_type *new_ast_func(char is_complete, char is_variadic, ast_data_t *ret, ast_sym_t *params){
    struct ast_func *func;
    union ast_type *node;

    func = calloc(1, sizeof(struct ast_func));
    func->is_complete = is_complete;
    func->is_variadic = is_variadic;
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

union ast_type *new_ast_sue(char data_type, char *name){
    struct ast_sue *sue;
    union ast_type *node;

    sue = calloc(1, sizeof(struct ast_sue));
    sue->data_type = data_type;
    sue->name = name;

    node = calloc(1, sizeof (union ast_type));
    node->sue = sue;
    return node;
}

union ast_type *new_ast_stru(char is_complete, ast_sym_t *tag, ast_sym_t *membs){
    struct ast_stru *stru;
    union ast_type *node;

    stru = calloc(1, sizeof(struct ast_stru));
    stru->is_complete = is_complete;
    stru->tag = tag;
    stru->membs = membs;

    node = calloc(1, sizeof (union ast_type));
    node->stru = stru;
    return node;
}

union ast_type *new_ast_unio(char is_complete, ast_sym_t *tag, ast_sym_t *membs){
    struct ast_unio *unio;
    union ast_type *node;

    unio = calloc(1, sizeof(struct ast_unio));
    unio->is_complete = is_complete;
    unio->tag = tag;
    unio->membs = membs;

    node = calloc(1, sizeof (union ast_type));
    node->unio = unio;
    return node;
}

union ast_type *new_ast_enu(char is_complete, ast_sym_t *tag){
    struct ast_enu *enu;
    union ast_type *node;
 
    enu = calloc(1, sizeof(struct ast_enu));
    enu->is_complete = is_complete;
    enu->tag = tag;

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
    data->node = node;
    return data;
}

ast_data_t *copy_ast_data(ast_data_t *data, int depth) {
    ast_data_t *copy;

    if (depth == 0) {
        return data;
    }

    copy = calloc(1, sizeof(ast_data_t));
    copy->size = data->size;
    copy->data_type = data->data_type;
    copy->qual = data->qual;
    switch (copy->data_type) {
        case DATA_SCAL:
            copy->node = new_ast_scal(
                data->node->scal->unsign,
                data->node->scal->scal_type);
            break;
        case DATA_PTR:
            copy->node = new_ast_ptr(
                copy_ast_data(data->node->ptr->to, depth-1));
            break;
        case DATA_ARY:
            copy->node = new_ast_ary(
                (TypedNumber) {{.i = data->node->ary->size}, TYPE_I},
                copy_ast_data(data->node->ary->elem, depth-1));
            break;
        case DATA_FUNC:
            copy->node = new_ast_func(
                data->node->func->is_complete,
                data->node->func->is_variadic,
                copy_ast_data(data->node->func->ret, depth-1),
                copy_sym_list(data->node->func->params));
            break;
        case DATA_PARAM:
            copy->node = new_ast_param(
                copy_ast_data(data->node->param->is, depth-1));
            break;
        case DATA_SUE:
            copy->node = new_ast_sue(
                data->node->sue->data_type,
                strdup(data->node->sue->name));
            break;
        case DATA_STRU:
            copy->node = new_ast_stru(
                data->node->stru->is_complete,
                data->node->stru->tag,
                copy_sym_list(data->node->stru->membs));
            break;
        case DATA_UNIO:
            copy->node = new_ast_unio(
                data->node->unio->is_complete,
                data->node->unio->tag,
                copy_sym_list(data->node->unio->membs));
            break;
        case DATA_ENU:
            copy->node = new_ast_enu(
                data->node->enu->is_complete,
                data->node->enu->tag);
            break;
        case DATA_LABEL:
            copy->node = new_ast_label(
                data->node->label->is_complete);
            break;
    }

    return copy;
}

int comp_set_max(int *src, int *targ, char *warn) {
    if (*targ && *src && *targ != *src) {
        /*ERROR incompatible types*/
        return -1;
    } else if (*targ != *src) {
        *warn = 1;
        if (*targ > *src) {
            *targ = *src;
        }
    }
    return 0;
}

ast_data_t *comb_ast_data(ast_data_t *old, ast_data_t *new) {
    char warn; // warn = 1 if by the end, we have implicit conversion
    ast_sym_t *new_param, *old_param;
    
    warn = 0;
    switch (new->data_type) {
        case DATA_SCAL: // h&s 5.11.1
            if (old->data_type != DATA_SCAL
            || new->node->scal->scal_type != old->node->scal->scal_type
            || new->node->scal->unsign != old->node->scal->unsign) {
                /*ERROR incompatible scalar types*/
                del_ast_data(new);
                return NULL;
            }
            break;
        case DATA_PTR:
            if (old->data_type != DATA_PTR
            || comb_ast_data(old->node->ptr->to, new->node->ptr->to) == NULL) {
                /*ERROR incompatible pointer types*/
                del_ast_data(new);
                return NULL;
            }
            break;
        case DATA_ARY: // h&s 5.11.3
            if (old->data_type != DATA_ARY
            || comp_set_max(&old->node->ary->size, &new->node->ary->size, &warn) == -1
            || comb_ast_data(old->node->ary->elem, new->node->ary->elem) == NULL) {
                /*ERROR incompatible array types*/
                del_ast_data(new);
                return NULL;
            }
            break;
        case DATA_FUNC: // h&s 5.11.4, prototype forms only
            if (new->node->func->is_complete == 1) {
                /*ERROR function already defined*/
                del_ast_data(new);
                return NULL;
            }

            if (old->data_type != DATA_FUNC
            || new->node->func->is_variadic != old->node->func->is_variadic
            || comb_ast_data(old->node->func->ret, new->node->func->ret) == NULL) {
                /*ERROR incompatible function types*/
                del_ast_data(new);
                return NULL;
            }

            old_param = old->node->func->params;
            new_param = new->node->func->params;
            while (old_param != NULL && new_param != NULL) {
                comb_ast_data(old_param->data, new_param->data);
                if (new_param->data == NULL) {
                    /*ERROR incompatible parameter types*/
                    del_ast_data(new);
                    return NULL;
                }
                new_param = new_param->prev;
                old_param = old_param->prev;
            }

            if (new_param != old_param) {
                /*ERROR incompatible function types, different number of parameters*/
                del_ast_data(new);
                return NULL;
            }
            break;
        case DATA_PARAM:
            if (old->data_type != DATA_PARAM
            || comb_ast_data(old->node->param->is, new->node->param->is) == NULL) {
                /*ERROR incompatible parameter types*/
                del_ast_data(new);
                return NULL;
            }
            break;
        case DATA_STRU:
            if (new->node->stru->tag != old->node->stru->tag) {
                /*ERROR incompatible struct types*/
                del_ast_data(new);
                return NULL;
            }
            break;
        case DATA_UNIO:
            if (new->node->unio->tag != old->node->unio->tag) {
                /*ERROR incompatible union types*/
                del_ast_data(new);
                return NULL;
            }
            break;
        case DATA_ENU: // h&s 5.11.2
            if (new->node->enu->tag != old->node->enu->tag) {
                /*ERROR conflicting enum types*/
                del_ast_data(new);
                return NULL;
            }
            break;
    }
    return new;
}

int del_ast_data(ast_data_t *data) {
    ast_sym_t *syms, *prev;

    syms = NULL;
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
            syms = data->node->func->params;
            break;
        case DATA_SUE:
            free(data->node->sue->name);
        case DATA_STRU:
            syms = data->node->stru->membs;
            break;
        case DATA_UNIO:
            syms = data->node->unio->membs;
            break;
        // DATA_SCAL, DATA_ENU, DATA_LABEL fall through
        // typedef case not implemented
    }

    while (syms != NULL) {
        prev = syms->prev;
        del_ast_sym(syms);
        syms = prev;
    }

    free(data->node);
    free(data);
    return 0;
}