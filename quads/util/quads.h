
#ifndef QUAD_H
#define QUAD_H

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <data.h>
#include <stdlib.h>
#include "table.h"
#include "expr.h"


// union generic_node{
//     int
// }
enum Q_TYPE { VAR, TMP, Q_CONST };

enum QUAD_CODES {
  // Simplest operations
  Q_ADD = 0,
  Q_SUB = 1,
  Q_MUL,
  Q_DIV,
  Q_MOD,

  // Binary & Shift operations
  // Q_SHL,
  // Q_SHR,

  // Q_BAND,
  // Q_BOR,
  // Q_XOR,

  // Q_LOGNOT,
  // Q_LOGOR,
  // Q_LOGAND,

  // Conditionals
  Q_CMP,
  //"Hidden" Quads
  // They evaluate to larger, related quads at generation time
  Q_EQUALS,
  Q_GREATER,
  Q_GEQ,
  Q_LEQ,
  Q_LESS,
  // Exposed versions
  L_EQ = 50,
  L_GR = 51,
  L_GEQ = 52,
  L_LEQ = 53,
  L_LT = 54,

  // STORE writes src2 into src1, (which should also be dest)
  Q_STORE,

  // Pointer dereferencing & UNOPS
  Q_LOAD,
  Q_MOV,

  // Function relatives
  Q_ARGBEGIN,
  Q_ARG,
  Q_CALL,

  // Branches
  Q_BREQ,
  Q_BRNE,

  // Just enough for now: Super basic

};
enum scope {
  LOCAL,
  PARAM,
  GLOBAL,
};

struct tmp {
  int value;
};

struct big_block typedef big_block;

union generic_node {
  char* v;
  struct tmp* t;
  // Assuming this is just an integer
  int c;
};

struct gen_node_t {
  enum Q_TYPE type;
  union generic_node data;
  ast_sym_t* symbol;
  //need a way to store symbol type
};

struct quad {
  int op;
  struct gen_node_t *destination, *src1, *src2;
} typedef quad;

// Singly linked list of quads or blocks
// If it's a block, the index get's used at print time

struct ll_nodes {
  quad* cur;
  struct ll_nodes* next;
};

struct quad_ll {
  struct ll_nodes* head;
  struct ll_nodes* tail;
};

struct big_block {
  // Array of pointers
  struct quad_ll quads_list;
  // int num_el;
  int block_ind;
  int func_ind;
  char* func_label;

} typedef big_block;


// A linked list of
struct CFG {
  big_block* block;
  struct CFG* true_exit;
  struct CFG* false_exit;
  //bool term block?
};

//As I don't have the symbol table to rely on, I'm doing this myself
//This is just an API reference, actual implementation would be in the parser

struct funct_decl {
  char* func_name;
  struct list_node* exprs_or_stmts;
};

// API
// When control flow logic is detected, generate quads for each of those trees,
// and then append them to a new big_block
//
//  struct big_block* generate_block(struct ast_sym func_base);

// This operates on a single AST, of undetermined type
// Count temps so they can tick up, clear when a single AST Node is complete

big_block* new_block();
struct big_block* descend_ast(ast_node* node, int* tmp_ctr, int* block_ctr,
                              big_block* parent);
struct quad_ll* descend_expr_ast(ast_node* node, struct quad_ll* list,
                                 int* tmp_ctr, ast_tab_t* table);
struct big_block* descend_stmt_ast(ast_node* node, int* tmp_ctr, int* block_ctr,
                                   big_block* parent);

#endif