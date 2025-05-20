#include "stmt.h"
#include "../parser.tab.h"
#include "yylval.h"

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

union ast_st *new_ast_if(ast_node *cond, ast_stmt_t *istmt, ast_stmt_t *estmt) {
    struct ast_if *ifst;
    union ast_st *st;

    ifst = calloc(1, sizeof (struct ast_if));
    ifst->cond = cond;
    ifst->istmt = istmt;
    ifst->estmt = estmt;

    st = calloc(1, sizeof(union ast_st));
    st->ifst = ifst;
    return st;
}

union ast_st *new_ast_for(ast_node *init, ast_node *cond, ast_node *adv, ast_stmt_t *stmt) {
    struct ast_for *forst;
    union ast_st *st;

    forst = calloc(1, sizeof (struct ast_for));
    forst->init = init;
    forst->cond = cond;
    forst->adv = adv;
    forst->stmt = stmt;

    st = calloc(1, sizeof(union ast_st));
    st->forst = forst;
    return st;
}

union ast_st *new_ast_while(ast_node *cond, ast_stmt_t *stmt) {
    struct ast_while *whilst;
    union ast_st *st;

    whilst = calloc(1, sizeof (struct ast_while));
    whilst->cond = cond;
    whilst->stmt = stmt;

    st = calloc(1, sizeof(union ast_st));
    st->whilst = whilst;
    return st;
}

union ast_st *new_ast_dowhile(ast_node *cond, ast_stmt_t *stmt) {
    struct ast_dowhile *dowhilst;
    union ast_st *st;

    dowhilst = calloc(1, sizeof (struct ast_dowhile));
    dowhilst->cond = cond;
    dowhilst->stmt = stmt;

    st = calloc(1, sizeof(union ast_st));
    st->dowhilst = dowhilst;
    return st;
}

union ast_st *new_ast_switch(ast_node *cond, ast_stmt_t *stmt) {
    struct ast_switch *switchst;
    union ast_st *st;

    switchst = calloc(1, sizeof (struct ast_switch));
    switchst->cond = cond;
    switchst->stmt = stmt;

    st = calloc(1, sizeof(union ast_st));
    st->switchst = switchst;
    return st;
}

union ast_st *new_ast_labelst(char label_type, ast_node *label, ast_stmt_t *stmt) {
    struct ast_labelst *labelst;
    union ast_st *st;

    labelst = calloc(1, sizeof (struct ast_labelst));
    labelst->label_type = label_type;
    labelst->label = label;
    labelst->stmt = stmt;

    st = calloc(1, sizeof(union ast_st));
    st->labelst = labelst;
    return st;
}

union ast_st *new_ast_goto(char *label) {
    struct ast_goto *gotost;
    union ast_st *st;

    gotost = calloc(1, sizeof (struct ast_goto));
    gotost->label = label;
    
    st = calloc(1, sizeof(union ast_st));
    st->gotost = gotost;
    return st;
}

union ast_st *new_ast_ret(ast_node *value) {
    struct ast_ret *ret;
    union ast_st *st;

    ret = calloc(1, sizeof (struct ast_ret));
    ret->value = value;
    
    st = calloc(1, sizeof(union ast_st));
    st->ret = ret;
    return st;
}

ast_stmt_t *new_ast_stmt(union ast_st *st, char stmt_type, ast_stmt_t *next) {
    ast_stmt_t *stmt;

    stmt = calloc(1, sizeof(ast_stmt_t));
    stmt->next = next;

    stmt->stmt_type = stmt_type;
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

    if (stmt == NULL) {
        return new;
    }
    
    temp = stmt;
    while (temp->next != NULL) {
        temp = temp->next;
    }

    temp->next = new;
    return stmt;
}