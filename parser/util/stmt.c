#include "stmt.h"

union ast_st *new_ast_expr(ast_node *expr) {
    union ast_st *st;

    st = calloc(1, sizeof(union ast_st));
    st->expr = expr;
    return st;
}

union ast_st *new_ast_block(ast_cpst_t *cpst) {
    union ast_st *st;
    
    st = calloc(1, sizeof(union ast_st));
    st->block = cpst;
    return st;
}

union ast_st *new_ast_if(ast_node *cond, ast_cpst_t *icpst, ast_cpst_t *ecpst) {
    struct ast_if *ifst;
    union ast_st *st;

    ifst = calloc(1, sizeof (struct ast_if));
    ifst->cond = cond;
    ifst->icpst = icpst;
    ifst->ecpst = ecpst;

    st = calloc(1, sizeof(union ast_st));
    st->ifst = ifst;
    return st;
}

union ast_st *new_ast_for(ast_node *init, ast_node *cond, ast_node *adv, ast_cpst_t *cpst) {
    struct ast_for *forst;
    union ast_st *st;

    forst = calloc(1, sizeof (struct ast_for));
    forst->init = init;
    forst->cond = cond;
    forst->adv = adv;
    forst->cpst = cpst;

    st = calloc(1, sizeof(union ast_st));
    st->forst = forst;
    return st;
}

union ast_st *new_ast_while(ast_node *cond, ast_cpst_t *cpst) {
    struct ast_while *whilst;
    union ast_st *st;

    whilst = calloc(1, sizeof (struct ast_while));
    whilst->cond = cond;
    whilst->cpst = cpst;

    st = calloc(1, sizeof(union ast_st));
    st->whilst = whilst;
    return st;
}

union ast_st *new_ast_dowhile(ast_node *cond, ast_cpst_t *cpst) {
    struct ast_dowhile *dowhilst;
    union ast_st *st;

    dowhilst = calloc(1, sizeof (struct ast_dowhile));
    dowhilst->cond = cond;
    dowhilst->cpst = cpst;

    st = calloc(1, sizeof(union ast_st));
    st->dowhilst = dowhilst;
    return st;
}

ast_stmt_t *new_ast_stmt(union ast_st *st, char stmt_type, ast_stmt_t *next) {
    ast_stmt_t *stmt;

    stmt = calloc(1, sizeof(ast_stmt_t));
    stmt->stmt_type = stmt_type;
    stmt->next = next;
    stmt->st = st;
    return stmt;
}

ast_cpst_t *new_ast_cpst(ast_stmt_t *stmt, ast_sym_t *sym) {
    ast_cpst_t *cpst;

    cpst = calloc(1, sizeof(ast_cpst_t));
    cpst->stmt = stmt;
    cpst->sym = sym;
    return cpst;
}

ast_stmt_t *append_stmt(ast_stmt_t *stmt, ast_stmt_t *new) {
    ast_stmt_t *temp;
    
    temp = stmt;
    if (temp == NULL) {
        stmt = new;
        return stmt;
    }
    
    while (temp->next != NULL) {
        temp = temp->next;
    }

    temp->next = new;
    return stmt;
}