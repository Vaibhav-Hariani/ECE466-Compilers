#include "stmt_out.h"

void print_stmt(ast_stmt_t *stmt, int num_spaces) {
    switch (stmt->stmt_type) {
        case STMT_EXPR:
            print_ast(stmt->st->expr, num_spaces + 1);
            break;
        case STMT_BLOCK:
            print_stmt_list(stmt->st->block->stmt, num_spaces, NULL);
            break;
        case STMT_IF:
            print_spaces(num_spaces);
            printf("IF:\n");
            print_spaces(num_spaces + 1);
            printf("COND:\n");
            print_ast(stmt->st->ifst->cond, num_spaces + 2);
            print_spaces(num_spaces + 1);
            printf("BODY:\n");
            print_stmt(stmt->st->ifst->istmt, num_spaces + 1);
            if (stmt->st->ifst->estmt != NULL) {
                print_spaces(num_spaces + 1);
                printf("ELSE:\n");
                print_stmt(stmt->st->ifst->estmt, num_spaces + 1);
            }
            break;
        case STMT_FOR:
            print_spaces(num_spaces);
            printf("FOR:\n");
            print_spaces(num_spaces + 1);
            printf("INIT:\n");
            print_ast(stmt->st->forst->init, num_spaces + 2);
            print_spaces(num_spaces + 1);
            printf("COND:\n");
            print_ast(stmt->st->forst->cond, num_spaces + 2);
            print_spaces(num_spaces + 1);
            printf("ADV:\n");
            print_ast(stmt->st->forst->adv, num_spaces + 2);
            print_spaces(num_spaces + 1);
            printf("BODY:\n");
            print_stmt(stmt->st->forst->stmt, num_spaces + 1);
            break;
        case STMT_WHILE:
            print_spaces(num_spaces);
            printf("WHILE:\n");
            print_spaces(num_spaces + 1);
            printf("COND:\n");
            print_ast(stmt->st->whilst->cond, num_spaces + 2);
            print_spaces(num_spaces + 1);
            printf("BODY:\n");
            print_stmt(stmt->st->whilst->stmt, num_spaces + 1);
            break;
        case STMT_DOWHILE:
            print_spaces(num_spaces);
            printf("DO:\n");
            print_spaces(num_spaces + 1);
            printf("BODY:\n");
            print_stmt(stmt->st->dowhilst->stmt, num_spaces + 1);
            print_spaces(num_spaces + 1);
            printf("COND:\n");
            print_ast(stmt->st->dowhilst->cond, num_spaces + 2);
            break;
        case STMT_SWITCH:
            print_spaces(num_spaces);
            printf("SWITCH:\n");
            print_spaces(num_spaces + 1);
            printf("COND:\n");
            print_ast(stmt->st->switchst->cond, num_spaces + 2);
            print_spaces(num_spaces + 1);
            printf("BODY:\n");
            print_stmt(stmt->st->switchst->stmt, num_spaces + 1);
            break;
        case STMT_CONTINUE:
            print_spaces(num_spaces);
            printf("CONTINUE\n");
            break;
        case STMT_BREAK:
            print_spaces(num_spaces);
            printf("BREAK\n");
            break;
        case STMT_CASE:
            print_spaces(num_spaces);
            printf("CASE:\n");
            print_spaces(num_spaces + 1);
            printf("COND:\n");
            print_ast(stmt->st->labelst->label, num_spaces + 2);
            print_spaces(num_spaces + 1);
            printf("BODY:\n");
            print_stmt(stmt->st->labelst->stmt, num_spaces + 2);
            break;
        case STMT_DEFAULT:
            print_spaces(num_spaces);
            printf("DEFAULT:\n");
            print_spaces(num_spaces + 1);
            printf("BODY:\n");
            print_stmt(stmt->st->labelst->stmt, num_spaces + 2);
            break;
        case STMT_LABELST:
            print_spaces(num_spaces);
            printf("LABELLED:\n");
            print_spaces(num_spaces + 1);
            printf("LABEL:\n");
            print_ast(stmt->st->labelst->label, num_spaces + 2);
            print_spaces(num_spaces + 1);
            printf("BODY:\n");
            print_stmt(stmt->st->labelst->stmt, num_spaces + 2);
            break;
            break;
        case STMT_GOTO:
            print_spaces(num_spaces);
            printf("GOTO\n");
            print_spaces(num_spaces + 1);
            printf("LABEL:\n");
            print_spaces(num_spaces + 2);
            printf("%s\n", stmt->st->gotost->label);
            break;
        case STMT_RETURN:
            print_spaces(num_spaces);
            printf("RETURN:\n");
            print_spaces(num_spaces + 1);
            printf("VALUE:\n");
            print_ast(stmt->st->ret->value, num_spaces + 2);
            break;
        case SYM_UNIO_T:
            fprintf(stderr, "i think you're lost\n");
            break;
    }
    printf("\n");
}

void print_stmt_list(ast_stmt_t *stmt, int num_spaces, char *name) {
    if (name) {
        print_spaces(num_spaces);
        printf("AST Dump for function %s:\n", name);
    }
    
    print_spaces(num_spaces + 1);
    printf("LIST {\n\n");
    while (stmt != NULL) {
        print_stmt(stmt, num_spaces + 4);
        stmt = stmt->next;
    }
    print_spaces(num_spaces + 1);
    printf("} ENDLIST\n");
}
