#include "symbol.h"
#include "symtab.tab.h"

ast_sym_t *new_ast_sym(char *name, char stg_type, char sym_type,
    ast_data_t *data, char *filename, int start){
    ast_sym_t *sym;

    sym = calloc(1, sizeof(ast_sym_t));
    sym->name = name;
    sym->stg_type = stg_type;
    sym->sym_type = sym_type;
    sym->data = data;
    sym->tail = data;
    sym->filename = filename;
    sym->start = start;
    return sym;
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

int struct_fix_memb(ast_data_t *data, ast_sym_t *memb) {
    int align, max_align;
    if (memb == NULL) {
        data->size = 0;
        return 0;
    }

    max_align = struct_fix_memb(memb->prev, data->node->stru->tag);
    align = get_align(memb, data->node->stru->tag);
    if (align > max_align) {
        max_align = align;
    }

    memb->offset = data->size += (align - ((data->size-1) % align + 1));
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

    union_fix_memb(memb->prev, data->node->stru->tag);
    align = get_align(memb, data->node->stru->tag);
    if (align > data->size) {
        data->size = align;
    }
    return 0;
}

int union_fix(ast_data_t *data) {
    return union_fix_memb(data, data->node->unio->membs);
}

ast_data_t *install_tail(ast_sym_t *sym, ast_data_t *tail) {
    if (sym->tail == NULL) {
        sym->data = tail;
        sym->tail = tail;
        return tail;
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