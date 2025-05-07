#include "symbol.h"

ast_sym_t *new_ast_sym(char *name, char stg_type, char sym_type,
        ast_data_t *data, char *filename, int start){
    ast_sym_t *sym;

    sym = calloc(1, sizeof(ast_sym_t));
    sym->name = name;
    sym->stg_type = stg_type;
    sym->sym_type = sym_type;
    sym->data = data;
    sym->tail = get_tail(data);
    sym->filename = filename;
    sym->start = start;
    return sym;
}

ast_sym_t *copy_ast_sym(ast_sym_t *sym) {
    ast_sym_t *copy;

    copy = new_ast_sym(strdup(sym->name), sym->stg_type, sym->sym_type,
        copy_ast_data(sym->data, -1), strdup(sym->filename), sym->start);
    copy->end = sym->end;
    copy->sco_type = sym->sco_type;
    copy->is_inline = sym->is_inline;
    copy->offset = sym->offset;
    return copy;
}

ast_sym_t *copy_sym_list(ast_sym_t *sym) {
    ast_sym_t *copy, *curr;

    curr = copy_ast_sym(sym);
    copy = curr;

    while (sym->prev != NULL) {
        sym = sym->prev;
        curr->prev = copy_ast_sym(sym);
        curr = curr->prev;
    }
    return copy;
}

ast_sym_t *del_ast_sym(ast_sym_t *sym) {
    ast_sym_t *prev;

    prev  = sym->prev;
    free(sym->name);
    free(sym->filename);
    del_ast_data(sym->data);
    
    free(sym);
    return prev;
}

ast_sym_t *list_start(ast_sym_t *sym) {
    while (sym->prev != NULL) {
        sym = sym->prev;
    }
    return sym;
}

int del_sym_list(ast_sym_t *sym) {
    while (sym != NULL) {
        sym = del_ast_sym(sym);
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

ast_sym_t *resolve_tag(ast_tab_t *tab, ast_sym_t *sym) {
    ast_sym_t *tag;

    tag = get_sym(tab, sym->tail->node->sue->name, NS_TAG, sym->start, sym->end);
    if (tag != NULL && tag->data->data_type == sym->data->node->sue->data_type) {
        sym->tail->data_type = tag->data->data_type;
        sym->tail->size = tag->data->size;

        free(sym->tail->node->sue->name);
        free(sym->tail->node->sue);
        free(sym->tail->node);

        sym->tail->node = tag->data->node;
        return tag;
    }
    return NULL;
}

int struct_fix_memb(ast_data_t *data, ast_sym_t *memb) {
    int pos_mod, align, max_align;
    if (memb == NULL) {
        data->size = 0;
        return 0;
    }

    memb->sym_type = SYM_STRU_M;
    max_align = struct_fix_memb(data, memb->prev);
    align = get_align(memb, data->node->stru->tag);
    if (align > max_align) {
        max_align = align;
    }

    pos_mod = (data->size-1) % align;
    pos_mod += (pos_mod < 0)? align : 0;
    memb->offset = data->size + (align - pos_mod - 1);
    data->size = memb->offset + memb->data->size;
    return max_align;
}

int struct_fix(ast_data_t *data) {
    int max_align;

    max_align = struct_fix_memb(data, data->node->stru->membs);

    // pad struct so members in struct arrays remain aligned
    data->size += max_align - ((data->size-1) % max_align + 1);
    return 0;
}

int union_fix_memb(ast_data_t *data, ast_sym_t *memb) {
    int align;
    if (memb == NULL) {
        data->size = 0;
        return 0;
    }

    memb->sym_type = SYM_UNIO_M;
    union_fix_memb(data, memb->prev);
    align = get_align(memb, data->node->unio->tag);
    if (align > data->size) {
        data->size = align;
    }
    return 0;
}

int union_fix(ast_data_t *data) {
    return union_fix_memb(data, data->node->unio->membs);
}

ast_data_t *get_tail(ast_data_t *data) {
    if (data == NULL) {
        return NULL;
    }
    switch(data->data_type) {
        case DATA_PTR:
            return (data->node->ptr->to == NULL)? data : get_tail(data->node->ptr->to);
        case DATA_ARY:
            return (data->node->ary->elem == NULL)? data : get_tail(data->node->ary->elem);
        case DATA_FUNC:
            return (data->node->func->ret == NULL)? data : get_tail(data->node->func->ret);
        default:
            return data;
    }
}

ast_data_t *install_tail(ast_sym_t *sym, ast_data_t *tail) {
    if (sym->tail == NULL) {
        sym->data = tail;
        sym->tail = get_tail(tail);
        return sym->tail;
    }

    switch (sym->tail->data_type) {
        case DATA_PTR:
            sym->tail->node->ptr->to = tail;
            sym->tail->size = sizeof (void *);
            break;
        case DATA_ARY:
            sym->tail->node->ary->elem = tail;
            sym->tail->size = tail->size * sym->tail->node->ary->size;
            break;
        case DATA_FUNC:
            sym->tail->node->func->ret = tail;
            break;
    }

    sym->tail = tail;
    return tail;
}