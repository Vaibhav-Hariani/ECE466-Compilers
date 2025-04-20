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

union ast_type *new_ast_func(char is_complete, ast_data_t *ret, ast_tab_t *params){
    struct ast_func *func;
    union ast_type *node;

    func = calloc(1, sizeof(struct ast_func));
    func->is_complete = is_complete;
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

union ast_type *new_ast_stru(char is_complete, ast_sym_t *tag, ast_tab_t *minitab){
    struct ast_stru *stru;
    union ast_type *node;

    stru = calloc(1, sizeof(struct ast_stru));
    stru->is_complete = is_complete;
    stru->tag = tag;
    stru->minitab = minitab;

    node = calloc(1, sizeof (union ast_type));
    node->stru = stru;
    return node;
}

union ast_type *new_ast_unio(char is_complete, ast_sym_t *tag, ast_tab_t *minitab){
    struct ast_unio *unio;
    union ast_type *node;

    unio = calloc(1, sizeof(struct ast_unio));
    unio->is_complete = is_complete;
    unio->tag = tag;
    unio->minitab = minitab;

    node = calloc(1, sizeof (union ast_type));
    node->unio = unio;
    return node;
}

union ast_type *new_ast_enu(ast_sym_t *tag){
    struct ast_enu *enu;
    union ast_type *node;
 
    enu = calloc(1, sizeof(struct ast_enu));
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
                copy_ast_data(data->node->ptr->to, depth-1)
            );
            break;
        case DATA_ARY:
            copy->node = new_ast_ary(
                (TypedNumber) {.type = TYPE_I, .val = data->node->ary->size},
                copy_ast_data(data->node->ary->elem, depth-1)
            );
            break;
        case DATA_FUNC:
            copy->node = new_ast_func(
                data->node->func->is_complete,
                copy_ast_data(data->node->func->ret, depth-1),
                copy_params(data->node->func->params)
            );
            break;
        case DATA_PARAM:
            copy->node = new_ast_param(
                copy_ast_data(data->node->param->is, depth-1)
            );
            break;
        case DATA_STRU:
            // tag intentionally not deep-copied
            // tab should be but isnt (would be important for flexible arrays)
            copy->node = new_ast_stru(
                data->node->stru->is_complete,
                data->node->stru->tag,
                data->node->stru->minitab
            );
            break;
        case DATA_UNIO:
            // same as above
            copy->node = new_ast_unio(
                data->node->unio->is_complete,
                data->node->unio->tag,
                data->node->unio->minitab
            );
            break;
        case DATA_ENU:
            copy->node = new_ast_enu(
                data->node->enu->is_complete
            );
            break;
        case DATA_LABEL:
            copy->node = new_ast_label(
                data->node->label->is_complete
            );
            break;
    }

    return copy;
}

ast_tab_t *copy_params(ast_tab_t *src) {
    ast_tab_t *copy;
    ast_sym_t *src_sym, **copy_curr;

    copy = new_ast_tab(src->parent, src->scope_type,
        src->filename, src->line);

    src_sym = src->misc;
    copy_curr = &copy->misc;
    while (src_sym != NULL) {
        *copy_curr = new_ast_sym(strdup(src_sym->name), src_sym->stg_type, src_sym->sym_type,
            src_sym->data, strdup(src_sym->filename), src_sym->line);
        copy_curr = &((*copy_curr)->next);
    }

    return copy;
}

int comp_set_max(int *src, int *targ, int *warn) {
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

ast_data_t *comb_ast_data(ast_data_t *src, ast_data_t *targ) {
    char warn; // warn = 1 if by the end, we have implicit conversion
    ast_data_t *comb, *targ_param, *src_param, *comb_param;
    ast_sym_t *comb_memb, *src_memb;
    
    warn = 0;
    comb = copy_ast_data(targ, 1);

    switch (comb->data_type) {
        case DATA_SCAL: // h&s 5.11.1
            if (src->data_type == DATA_ENU && comb->node->scal->scal_type == SCAL_INT) {
                // nonstandard but mentioned in h&s 5.11.2, works when enums, ints same
                // same condition reflected in DATA_ENU case below
                break; // no error
            }
            if (src->data_type != DATA_SCAL
            || comb->node->scal->scal_type != src->node->scal->scal_type
            || comb->node->scal->unsign != src->node->scal->unsign) {
                /*ERROR incompatible scalar types*/
                del_ast_data(comb);
                return NULL;
            }
            break;
        case DATA_PTR:
            if (src->data_type != DATA_PTR
            || (comb->node->ptr->to = comb_ast_data(src->node->ptr->to, comb->node->ptr->to)) == NULL) {
                /*ERROR incompatible pointer types*/
                del_ast_data(comb);
                return NULL;
            }
            break;
        case DATA_ARY: // h&s 5.11.3
            if (src->data_type != DATA_ARY
            || comp_set_max(&src->node->ary->size, &targ->node->ary->size, &warn) == -1
            || (comb->node->ary->elem = comb_ast_data(src->node->ary->elem, comb->node->ary->elem)) == NULL) {
                /*ERROR incompatible array types*/
                del_ast_data(comb);
                return NULL;
            }
            break;
        case DATA_FUNC: // h&s 5.11.4, prototype forms only
            if (src->data_type != DATA_FUNC
            || comb->node->func->params->scope_type != src->node->func->params->scope_type
            || (comb->node->func->ret = comb_ast_data(src->node->func->ret, comb->node->func->ret)) == NULL) {
                /*ERROR incompatible function types*/
                del_ast_data(comb);
                return NULL;
            }

            comb_memb = comb->node->func->params->memb;
            src_memb = src->node->func->params->memb;
            while (comb_memb != NULL && src_memb != NULL) {
                comb_param = comb_ast_data(comb->node->func->params->memb->data, src->node->func->params->memb->data);
                if (comb_param == NULL) {
                    /*ERROR incompatible parameter types*/
                    del_ast_data(comb);
                    return NULL;
                }
                comb->node->func->params->memb->data = comb_param;
                comb_memb = comb_memb->next;
                src_memb = src_memb->next;
            }

            if (comb_memb != src_memb) {
                /*ERROR incompatible function types, different number of parameters*/
                del_ast_data(comb);
                return NULL;
            }
            break;
        case DATA_PARAM:
            if (src->data_type != DATA_PARAM
            || (comb->node->param->is = comb_ast_data(src->node->param->is, comb->node->param->is)) == NULL) {
                /*ERROR incompatible parameter types*/
                del_ast_data(comb);
                return NULL;
            }
            break;
        case DATA_STRU:
            if (comb->node->stru->tag != src->node->stru->tag) {
                /*ERROR incompatible struct types*/
                del_ast_data(comb);
                return NULL;
            }
            break;
        case DATA_UNIO:
            if (comb->node->unio->tag != src->node->unio->tag) {
                /*ERROR incompatible union types*/
                del_ast_data(comb);
                return NULL;
            }
            break;
        case DATA_ENU: // h&s 5.11.2
            if (src->data_type != DATA_ENU
            && !(src->data_type == DATA_SCAL && src->node->scal->scal_type == SCAL_INT)) {
                /*ERROR type incompatible with enum*/
                del_ast_data(comb);
                return NULL;
            }
            break;
    }
    
    return comb;
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
        // DATA_SCAL, DATA_ENU, DATA_LABEL fall through
        // typedef case not implemented
    }

    free(data->node);
    free(data);
    return 0;
}

ast_sym_t *new_ast_sym(char *name, char stg_type, char sym_type,
    ast_data_t *data, char *filename, int line){
    ast_sym_t *sym;

    sym = calloc(1, sizeof(ast_sym_t));
    sym->name = name;
    sym->stg_type = stg_type;
    sym->sym_type = sym_type;
    sym->data = data;
    sym->tail = data;
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

    curr = tab;
    while (curr != NULL) {
        switch (sym_type) {
            /*misc*/
            case SYM_VAR:
            case SYM_TYPEDEF:
            case SYM_FUNC:
            case SYM_ENU_C:
            case SYM_PARAM:
                sym = curr->misc;
                break;
            /*tag*/
            case SYM_STRU_T:
            case SYM_UNIO_T:
            case SYM_ENU_T:
                sym = curr->tag;
                break;
            /*member*/
            case SYM_STRU_M:
            case SYM_UNIO_M:
                sym = curr->memb;
                break;
            /*label*/
            case SYM_LABEL:
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
    ast_data_t *comb;
    ast_sym_t *old, *next;
    ast_tab_t *curr_tab;
    int fails;

    fails = 0;
    while (sym != NULL) {
        curr_tab = tab;
        next = sym->next;
        sym->next = NULL;
        old = lookup(tab, sym->name, sym->sym_type);
        if (old != NULL && old->tab == tab) {
            if (replace_dup) {
                // external redefinitions h&s 4.2.5
                if ((old->stg_type == STG_EXTERN_EXP || old->stg_type == STG_EXTERN_IMP)
                && (old->stg_type == STG_EXTERN_EXP || old->stg_type == STG_EXTERN_IMP)
                && (comb = comb_ast_data(sym->data, old->data)) != NULL) {
                    del_ast_data(old->data);
                    old->data = comb;
                    free(old->filename);
                    old->filename = strdup(sym->filename);
                    old->line = sym->line;
                    del_ast_sym(sym);
                } else {
                        /*ERROR conflicting types*/
                        del_ast_sym(sym);
                        fails++;
                }
            } else {
                // ERROR symbol already defined, cant replace
                del_ast_sym(sym);
                fails++;
            }
        } else {
            switch (sym->sym_type) {
                /*misc*/
                case SYM_VAR:
                case SYM_TYPEDEF:
                case SYM_FUNC:
                case SYM_ENU_C:
                case SYM_PARAM:
                    sym->tab = curr_tab;
                    sym->next = curr_tab->misc;
                    curr_tab->misc = sym;
                    break;

                /*tag*/
                case SYM_STRU_T:
                case SYM_UNIO_T:
                case SYM_ENU_T:
                    sym->tab = curr_tab;
                    sym->next = curr_tab->tag;
                    curr_tab->tag = sym;
                    break;

                /*member*/
                case SYM_STRU_M:
                case SYM_UNIO_M:
                    if (sym->sym_type == SYM_STRU_M) {
                        /*struct/union def within struct/union takes enclosing scope*/
                        while (curr_tab->scope_type == SCOPE_STRUNIO) {
                            curr_tab = tab->parent;
                        }
                    }
                    fails += enter(curr_tab, sym, replace_dup);
                    break;

                /*label*/
                case SYM_LABEL:
                    sym->tab = curr_tab;
                    sym->next = curr_tab->label;
                    curr_tab->label = sym;
                    break;
            }
        }
        sym = next;
    }

    return 0;
}
    

int get_align(ast_sym_t *memb, ast_sym_t *sym) {
    int align, temp;

    align = 1;
    switch(memb->data->data_type) {
        // bitfields not implemented
        case DATA_SCAL:
            switch (memb->data->node->scal->scal_type) {
                case SCAL_SHORT:
                    align = __alignof__ (short);
                    break;
                case SCAL_INT:
                    align = __alignof__ (int);
                    break;
                case SCAL_LONG:
                    align = __alignof__ (long);
                    break;
                case SCAL_LONGLONG:
                    align = __alignof__ (long long);
                    break;
                case SCAL_FLOAT:
                    align = __alignof__ (float);
                    break;
                case SCAL_DOUB:
                    align = __alignof__ (double);
                    break;
                case SCAL_LONGDOUB:
                    align = __alignof__ (long double);
                    break;
                case SCAL_CHAR:
                    align = __alignof__ (char);
                    break;
            }
        case DATA_PTR:
            align = __alignof__ (void *);
            break;
        case DATA_STRU: case DATA_UNIO:
            if (!strcmp(memb->name, sym->name)) {
                /*ERROR cannot be own type*/
            }

        case DATA_ARY: // DATA_STRU and DATA_UNIO fall through
            // guess: sizeof type rounded up to nearest power of 2
            temp = memb->data->size;
            while (align < temp) {
                align *= 2;
            }
            break;
        case DATA_ENU:
            align = __alignof__ (int);
            break;
        default:
            /*ERROR type not permitted as struct member*/
            break;
    }

    return align;
}

int struct_fix(ast_data_t *data) {
    int size, align, max_align;
    ast_sym_t *curr;

    size = 0;
    max_align = 0;
    curr = data->node->stru->minitab->memb;
    while (curr != NULL) {
        align = get_align(curr, data->node->stru->tag);

        if (align >= max_align) {
            max_align = align;
        }

        //             size + padding to reach alignment 
        curr->offset = size += (align - ((size-1) % align + 1));
        size = curr->offset + curr->data->size;
    }

    // pad struct so members in struct arrays remain aligned
    data->size = size + (max_align - ((size-1) % max_align + 1));
    return 0;
}

int union_fix(ast_data_t *data) {
    int size, align;
    ast_sym_t *curr;

    size = 0;
    curr = data->node->unio->minitab->memb;
    while (curr != NULL) {
        align = get_align(curr, data->node->unio->tag);

        if (align >= size) {
            size = align;
        }
    }

    data->size = size;
    return 0;
}

ast_data_t *install_tail(ast_data_t *data, ast_data_t *tail) {
    switch (data->data_type) {
        case DATA_PTR:
            data->node->ptr->to = tail;
            break;
        case DATA_ARY:
            data->node->ary->elem = tail;
            break;
        case DATA_FUNC:
            data->node->func->ret = tail;
            break;
    }

    return tail;
}

ast_data_t *merge_types(ast_data_t *spec, ast_data_t *data) {
    return NULL;
}