#include "table.h"

int get_namespace(int sym_type) {
    switch (sym_type) {
        case SYM_VAR:
        case SYM_TYPEDEF:
        case SYM_FUNC:
        case SYM_ENU_C:
        case SYM_PARAM:
            return NS_MISC;
        case SYM_STRU_T:
        case SYM_UNIO_T:
        case SYM_ENU_T:
            return NS_TAG;
        case SYM_STRU_M:
        case SYM_UNIO_M:
            return NS_MEMB;
        case SYM_LABEL:
            return NS_LABEL;
    }
    return -1;
}

ast_tab_t *new_table(unsigned int min_size) {
    ast_tab_t *tab;
    
    tab = calloc(1, sizeof (ast_tab_t));
    tab->filled = 0;
    tab->size = sizeup(min_size);
    tab->cells = calloc(tab->size, sizeof (ast_cell_t *));
    return tab;
}

int insert(ast_tab_t *tab, ast_sym_t *sym, char sco_type, int end, char replace_dup) {
    int i;
    ast_data_t *comb;
    ast_sym_t *tag;

    if (tab->filled * 2 >= tab->size) {
        if (rehash(tab) == -1) {
            return 2;
        }
    }

    sym->sco_type = sco_type; // symbol's scope
    sym->end = end; // end of the symbol's scope

    // filters out anonymous struct/union/enums
    if (get_namespace(sym->sym_type) == NS_TAG && sym->name == NULL) {
        return 0;
    }

    // resolves placeholder struct/union/enum tags,
    // inserts incomplete symbol into table if need be
    if (sym->tail->data_type == DATA_SUE
    && sym->tail->node->sue->name != NULL) {

        // insert new incomplete type if struct/union/enum tag not resolved
        if (resolve_tag(tab, sym) == NULL) {
            switch (sym->tail->node->sue->data_type) {
                case DATA_STRU:
                    tag = new_ast_sym(sym->tail->node->sue->name, STG_NA, SYM_STRU_T, 
                        new_ast_data(0, DATA_STRU, QUAL_NONE,
                            new_ast_stru(0, NULL, NULL)),
                        sym->filename, end);
                    tag->data->node->stru->tag = tag;
                    break;
                case DATA_UNIO:
                    tag = new_ast_sym(sym->tail->node->sue->name, STG_NA, SYM_UNIO_T, 
                        new_ast_data(0, DATA_UNIO, QUAL_NONE,
                            new_ast_unio(0, NULL, NULL)),
                        sym->filename, end);
                    tag->data->node->unio->tag = tag;
                    break;
                case DATA_ENU:
                    tag = new_ast_sym(sym->tail->node->sue->name, STG_NA, SYM_ENU_T, 
                        new_ast_data(0, DATA_ENU, QUAL_NONE,
                            new_ast_enu(0, NULL)),
                        sym->filename, end);
                    tag->data->node->enu->tag = tag;
                    break;
            }
            insert(tab, tag, sco_type, end, replace_dup);
        }
    }

    i = hash(tab, sym->name, get_namespace(sym->sym_type));
    while (tab->cells[i] != NULL) {
        if (!strcmp(sym->name, tab->cells[i]->sym->name)
        && get_namespace(tab->cells[i]->sym->sym_type) == get_namespace(sym->sym_type)
        && sym->end == tab->cells[i]->sym->end) {
            
            // sym is a definition for an incomplete struct/union/enum
            if ((tab->cells[i]->sym->sym_type == SYM_STRU_T
                && sym->sym_type == SYM_STRU_T
                && tab->cells[i]->sym->data->node->stru->is_complete == 0)
            ||  (tab->cells[i]->sym->sym_type == SYM_UNIO_T
                && sym->sym_type == SYM_UNIO_T
                && tab->cells[i]->sym->data->node->unio->is_complete == 0)
            ||  (tab->cells[i]->sym->sym_type == SYM_ENU_T
                && sym->sym_type == SYM_ENU_T
                && tab->cells[i]->sym->data->node->enu->is_complete == 0)) {
                del_ast_sym(tab->cells[i]->sym);
                tab->cells[i]->sym = sym;
                print_obj_def(sym, 0);
                return 0;
            }

            if (replace_dup) {
                // external redefinitions h&s 4.2.5
                if ((tab->cells[i]->sym->stg_type == STG_EXTERN_EXP ||
                    tab->cells[i]->sym->stg_type == STG_EXTERN_IMP)
                && (comb = comb_ast_data(sym->data, tab->cells[i]->sym->data)) != NULL) {
                    del_ast_data(tab->cells[i]->sym->data);
                    tab->cells[i]->sym->data = comb;
                    free(tab->cells[i]->sym->filename);
                    tab->cells[i]->sym->filename = strdup(sym->filename);
                    tab->cells[i]->sym->start = sym->start;
                    del_ast_sym(sym);
                } else {
                        /*ERROR conflicting types*/
                        del_ast_sym(sym);
                        return 1;
                }
            } else {
                // ERROR symbol already defined, cant replace
                del_ast_sym(sym);
                return 1;
            }
        }
        i = (i == tab->size - 1)? 0 : i + 1;
    }

    tab->cells[i] = calloc(1, sizeof (ast_cell_t));
    tab->cells[i]->sym = sym;
    tab->filled++;

    // output
    switch (sym->sym_type) {
        case SYM_VAR:
            print_sym_decl(sym, 0);
            break;
        case SYM_FUNC:
            if (sym->data->node->func->is_complete) {
                print_sym_decl(sym, 0);
            }
        case SYM_STRU_T:
            if (sym->data->node->stru->is_complete) {
                print_obj_def(sym, 0);
            }
            break;
        case SYM_UNIO_T:
            if (sym->data->node->unio->is_complete) {
                print_obj_def(sym, 0);
            }
            break;
    }

    return 0;
}

int insert_list(ast_tab_t *tab, ast_sym_t *sym, char sco_type, int end, char replace_dup) {
    int fails;

    if (sym == NULL) {
        return 0;
    }

    fails = insert(tab, sym->prev, sco_type, end, replace_dup);
    fails += (insert(tab, sym, sco_type, end, replace_dup) != 0);

    return fails;
}

int contains(ast_tab_t *tab, char *key, char namespace, int start, int end) {
    return (locate(tab, key, namespace, start, end) != -1);
}

ast_sym_t *get_sym(ast_tab_t *tab, char *key, char namespace, int start, int end) {
    int i;

    i = locate(tab, key, namespace, start, end);
    if (i == -1) {
        return NULL;
    }
    return tab->cells[i]->sym;
}

int set_sym(ast_tab_t *tab, ast_sym_t *sym) {
    int i;

    i = locate(tab, sym->name, get_namespace(sym->sym_type), sym->start, sym->end);
    if (i == -1) {
        return -1;
    }
    del_ast_sym(tab->cells[i]->sym);
    tab->cells[i]->sym = sym;
    return 0;
}

int remove_sym(ast_tab_t *tab, char *key, char namespace, int start, int end) {
    int i;

    i = locate(tab, key, namespace, start, end);
    if (i == -1) {
        return -1;
    }
    tab->cells[i]->is_deleted = 1;
    return 0;
}

int del_table(ast_tab_t *tab) {
    int i;

    for (i = 0; i < tab->size; i++) {
        if (tab->cells[i] != NULL) {
            del_ast_sym((tab->cells[i])->sym);
            free(tab->cells[i]);
        }
    }

    free(tab->cells);
    return 0;
}

int sizeup(unsigned int size) {
    int i;
    unsigned int primes[21] = {
        5051, 10111, 19483, 40697,
        82883, 166301, 334751, 671717,
        1376161, 2836919, 5749879, 11578393,
        23828443, 47653831, 95730181, 191884001,
        395567321, 801795227, 1637744677, 2711295017,
        4294967291 };

    for (i = 0; i < 21 && size > primes[i]; i++);
    if (i == 21) {
        return -1;
    }

    return primes[i];
}

unsigned int hash(ast_tab_t *tab, char *key, char namespace) {
    unsigned int hash_val;
    int i;

    hash_val = 0;
    for (i = 0; key[i] != '\0'; i++) {
        hash_val = 37 * hash_val + key[i];
    }

    hash_val = 43 * (hash_val + 19 * namespace);

    return hash_val % tab->size; // this hash function is very  bad
}

unsigned int locate(ast_tab_t *tab, char *key, char namespace, int start, int end) {
    int i, ret;

    ret = -1;
    i = hash(tab, key, namespace);
    while (tab->cells[i] != NULL) {
        if (!strcmp(key, tab->cells[i]->sym->name)  // same key
        && get_namespace(tab->cells[i]->sym->sym_type) == namespace // same namespace
        && tab->cells[i]->sym->start <= start   // scope is or contains the scope
        && tab->cells[i]->sym->end >= end) {    // specified by start and end
            if (ret == -1) {
                ret = i;
            } else if (tab->cells[i]->sym->start > tab->cells[ret]->sym->start) {
                ret = i;
            }
        }
        i = (i == tab->size - 1)? 0 : i + 1;
    }
    return ret;
}

int rehash(ast_tab_t *tab) {
    int i, old_size;
    ast_cell_t **old_cells;

    errno = 0;
    old_size = tab->size;
    old_cells = calloc(old_size, sizeof (ast_cell_t *));
    if (errno != 0) {
        /*ERROR out of memory*/
        return -1;
    }

    for (i = 0; i < tab->size; i++) {
        if (tab->cells[i] != NULL) {
            if (!(tab->cells[i])->is_deleted) {
                old_cells[i] = calloc(1, sizeof (ast_cell_t));
                old_cells[i]->sym = (tab->cells[i])->sym;
            } else {
                del_ast_sym((tab->cells[i])->sym);
            }
            (tab->cells[i])->is_deleted = 0;
            (tab->cells[i])->sym = NULL;
            free(tab->cells[i]);
        }
    }

    if ((tab->size = sizeup(tab->size)) == -1) {
        /*ERROR maximum size reached*/
        return -1;
    }

    tab->cells = reallocarray(tab->cells, tab->size, sizeof (ast_cell_t *));
    if (errno != 0) {
        /*ERROR out of memory*/
        return -1;
    }

    for (i = 0; i < old_size; i++) {
        if (old_cells[i] != NULL) {
            insert(tab, old_cells[i]->sym, old_cells[i]->sym->end, old_cells[i]->sym->sco_type, 0);
            // insert from old_cells to tab->cells
        }
    }

    return 0;
}
