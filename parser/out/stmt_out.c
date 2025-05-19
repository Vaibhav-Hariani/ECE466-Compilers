#include "stmt_out.h"

void print_stmt(ast_stmt_t *stmt) {
    switch (stmt->stmt_type) {
        case STMT_EXPR:
            print_ast(stmt->st->expr);
            break;
        case STMT_BLOCK:
            fprintf(stderr, "{ ENTER BLOCK\n");
            print_stmt_list(stmt->st->block->stmt);
            fprintf(stderr, "} LEAVE BLOCK\n");
            break;
        case STMT_IF:
        case STMT_FOR:
        case STMT_WHILE:
        case STMT_DOWHILE:
        case STMT_SWITCH:
            /*not implemented*/
            break;
        case SYM_UNIO_T:
            fprintf(stderr, "i think you're lost\n");
            break;
    }
    fprintf(stderr, "\n");
}

void print_stmt_list(ast_stmt_t *stmt) {
    while (stmt != NULL) {
        print_stmt(stmt);
        stmt = stmt->next;
    }
}
