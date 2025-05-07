#include "symtab_output.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "symtab.tab.h"

void print_indent(int num_tabs) {
    char tab_arr[num_tabs + 1];
    memset(tab_arr, '\t', num_tabs);
    tab_arr[num_tabs] = '\0';
    printf("%s", tab_arr);
}

int print_qual(char qual) {
    if (qual & QUAL_CONST) {
        printf("const ");
    } if (qual & QUAL_VOLATILE) {
        printf("volatile ");
    } if (qual & QUAL_RESTRICT) {
        printf("restrict ");
    }
    return 0;
}

int print_scal(char unsign, char scal_type) {
    if (unsign == 1) {
        printf("unsigned ");
    } else if (unsign == -1) { // signed char
        printf("signed ");
    }
    switch (scal_type) {
        // we like to have a little bit of fun around here
        case SCAL_LONGLONG:
            printf("long "); 
        case SCAL_LONG:
            printf("long ");
        case SCAL_INT:
            printf("int\n");
            break;
        case SCAL_SHORT:
            printf("short int");
            break;
        case SCAL_LONGDOUB:
            printf("long ");
        case SCAL_DOUB:
            printf("double\n");
            break;
        case SCAL_FLOAT:
            printf("float\n");
            break;
        case SCAL_CHAR:
            printf("char\n");
            break;
        case SCAL_VOID:
            printf("void\n");
            break;
    }
    return 0;
}

int print_scope(int sco_type, char *filename, int line) {
    switch (sco_type) {
        case SCO_FILE:
            printf("global ");
            break;
        case SCO_FUNC:
            printf("function ");
            break;
        case SCO_BLOCK:
            printf("block ");
            break;
        case SCO_PROTO:
            printf("prototype ");
            break;
        case SCO_STRUNIO:
            printf("struct/union ");
            break;
        case SCO_ENU:
            printf("enum ");
            break;
    }
    printf("starting at <%s>:%d\n", filename, line);
    return 0;
}

int print_stg(int stg_type) {
    switch (stg_type) {
        case STG_EXTERN_IMP:
            printf("implicit extern\n");
            break;
        case STG_EXTERN_EXP:
            printf("explicit extern\n");
            break;
        case STG_STATIC:
            printf("static\n");
            break;
        case STG_REGISTER:
            printf("register\n");
            break;
        case STG_AUTO_LOC:
            printf("local auto\n");
            break;
        case STG_AUTO_PAR:
            printf("parameter auto\n");
            break;
        case STG_NA:
            printf("N/A\n");
            break;
    }
    return 0;
}

int print_params(ast_sym_t *params, int num_tabs) {
    if (params == NULL) {
        return 0;
    }

    print_params(params->prev, num_tabs);

    print_indent(num_tabs);
    printf("parameter %s {\n", params->name);
    print_indent(num_tabs + 1);
    printf("stg class: \t");
    print_stg(params->stg_type);
    print_indent(num_tabs + 1);
    printf("data type:\n");
    print_data(params->data, num_tabs+2);
    print_indent(num_tabs);
    printf("}\n");

    return 0;
}

int print_data(ast_data_t *data, int num_tabs) {
    print_indent(num_tabs);
    print_qual(data->qual);
    switch (data->data_type) {
        case DATA_SCAL:
            print_scal(data->node->scal->unsign, data->node->scal->scal_type);
            break;
        case DATA_PTR:
            printf("pointer to\n");
            print_data(data->node->ptr->to, num_tabs + 1);
            break;
        case DATA_ARY:
            if (data->node->ary->size > 0) {
                printf("ary of %d elements of type\n", data->node->ary->size);
            } else {
                printf("ary of elements of type\n");
            }
            print_data(data->node->ary->elem, num_tabs + 1);
            break;
        case DATA_FUNC:
            if (!data->node->func->is_complete) {
                printf("incomplete ");
            }
            printf("function with return type\n");
            print_data(data->node->func->ret, num_tabs + 1);
            print_indent(num_tabs);
            printf("which takes the arguments:");
            print_params(data->node->func->params, num_tabs + 1);
            break;
        case DATA_PARAM:
            printf("parameter of type\n");
            print_data(data->node->param->is, num_tabs = 1);
            break;
        case DATA_STRU:
            if (data->node->stru->tag->name != NULL) {
                printf("struct %s ", data->node->stru->tag->name);
            } else {
                printf("anonymous struct ");
            }
            if (data->node->stru->is_complete) {
                printf("(defined at <%s>:%d)\n",
                    data->node->stru->tag->filename,
                    data->node->stru->tag->start);
            } else {
                printf("(incomplete)\n");
            }
            break;
        case DATA_UNIO:
            if (data->node->unio->tag->name != NULL) {
                printf("union %s ", data->node->unio->tag->name);
            } else {
                printf("anonymous union ");
            }
            if (data->node->unio->is_complete) {
                printf("(defined at <%s>:%d)\n",
                    data->node->unio->tag->filename,
                    data->node->unio->tag->start);
            } else {
                printf("(incomplete)\n");
            }
            break;
        case DATA_ENU:
            if (data->node->enu->tag->name != NULL) {
                printf("enum %s ", data->node->enu->tag->name);
            } else {
                printf("anonymous enum ");
            }
            printf("(defined at <%s>:%d)\n",
                    data->node->unio->tag->filename,
                    data->node->unio->tag->start);
            break;
        case DATA_LABEL:
            printf("label");
            break;
    }
    return 0;
}

int print_memb_decl(ast_sym_t *memb, ast_sym_t *sym, int num_tabs, char *data_name) {
    if (memb == NULL) {
        return 0;
    }

    print_memb_decl(memb->prev, sym, num_tabs, data_name);

    print_indent(num_tabs);
    printf("%s %s field %s defined at <%s>:%d {\n",
        data_name, sym->name, memb->name, memb->filename, memb->start);
    print_indent(num_tabs + 1);
    printf("scope: ");
    print_scope(memb->sco_type, memb->filename, memb->start);
    print_indent(num_tabs + 1);
    printf("data type:\n");
    print_data(memb->data, num_tabs + 2);
    print_indent(num_tabs + 1);
    if (sym->sym_type == SYM_STRU_T) {
        printf("offset:\n");
        print_indent(num_tabs + 2);
        printf("%d\n", memb->offset);
        print_indent(num_tabs);
    }
    printf("}\n");
    return 0;
}

int print_sym_decl(ast_sym_t *sym, int num_tabs) {
    print_indent(num_tabs);
    printf("%s %s defined at <%s>:%d {\n",
        (sym->sym_type == SYM_VAR)? "variable" : "function",
        sym->name, sym->filename, sym->start);
    print_indent(num_tabs + 1);
    printf("scope:\n");
    print_indent(num_tabs + 2);
    print_scope(sym->sco_type, sym->filename, sym->start);
    print_indent(num_tabs + 1);
    printf("stg class:\n");
    print_indent(num_tabs + 2);
    print_stg(sym->stg_type);
    print_indent(num_tabs + 1);
    printf("data type:\n");
    print_data(sym->data, num_tabs+2);
    print_indent(num_tabs + 1);
    printf("size:\n");
    print_indent(num_tabs + 2);
    printf("%d\n", sym->data->size);
    print_indent(num_tabs);
    printf("}\n\n");
    return 0;
}

int print_obj_def(ast_sym_t *sym, int num_tabs) {
    char *data_name;
    ast_sym_t *membs;

    membs = NULL;
    data_name = NULL;
    switch (sym->sym_type) {
        case SYM_STRU_T:
            data_name = strdup("struct");
            membs = sym->data->node->stru->membs;
            break;
        case SYM_UNIO_T:
            data_name = strdup("union");
            membs = sym->data->node->unio->membs;
            break;
    }
    
    print_indent(num_tabs);
    printf("%s %s defined at <%s>:%d {\n",
        data_name, sym->name, sym->filename, sym->start);
    print_indent(num_tabs + 1);
    printf("scope: ");
    print_scope(sym->sco_type, sym->filename, sym->start);
    
    print_memb_decl(membs, sym, num_tabs + 1, data_name);

    free(data_name);
    print_indent(num_tabs);
    printf("}\n\n");

    return 0;
}
