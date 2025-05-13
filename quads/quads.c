#include "quads.h"

#define new_quad calloc(1, sizeof(quad))

big_block* new_block() {
  big_block* new_block = calloc(1, sizeof(big_block));
  new_block->quad_head = calloc(1, sizeof(struct quad_ll));
}
// Creates a tmp node, and increments tmp_ctr
struct gen_node_t* new_tmp(int tmp_ctr) {
  struct gen_node_t* node = calloc(1, sizeof(struct gen_node_t));
  node->type = TMP;
  struct tmp* data = calloc(1, sizeof(struct tmp));
  data->value = tmp_ctr;
  node->data.t = data;
  return node;
}

struct gen_node_t* new_var(struct ident* d) {
  struct gen_node_t* node = calloc(1, sizeof(struct gen_node_t));
  node->type = VAR;
  node->data.t = d;
  return node;
}

struct gen_node_t* new_const(int val) {
  struct gen_node_t* node = calloc(1, sizeof(struct gen_node_t));
  node->type = CONST;
  node->data.c = val;
  return node;
}

quad* quad_gen(struct gen_node_t* dest, struct gen_node_t* src1,
               struct gen_node_t* src2, int quad_code) {
  quad* quad = new_quad;
  quad->destination = dest;
  quad->op = quad_code;
  quad->src1 = src1;
  quad->src2 = src2;
  return quad;
}

// Appends the quads from new to the end of ref.
struct gen_node_t* append(big_block* ref, big_block* new) {
  ref->quad_tail->next = new->quad_head;
  ref->quad_tail = new->quad_tail;
  ref->num_el += new->num_el;
  // We can safely get rid of new
  // This might be the only memory control in this compiler
  free(new);
  return ref->quad_tail->cur;
}

// Structured like this for more of an API interface
// Underlying data structue changed a lot during development
void append_quad(big_block* ref, quad* new) {
  struct quad_ll* ll_wrapper = calloc(1, sizeof(struct quad_ll));
  ll_wrapper->cur = new;
  ref->quad_tail->next = ll_wrapper;
  ref->quad_tail = ll_wrapper;
}

struct gen_node_t* get_element(ast_node* node, big_block* ref, int* tmp_ctr) {
  // If this is not a constant or an expression
  if (node == NULL) {
    return NULL;
  }
  if (node->type == AST_ident) {
    return new_var(node->obj.ident);
  }
  if (node->type == AST_num) {
    return new_const(node->obj.num.val.i);
  }

  big_block* b1 = descend_ast(node, tmp_ctr);
  return append(ref, b1);
}

int parser_quad_op_conv(int parser_code){


  return parser_code - Q_ADD;
};

// For single op instructions, a destination may be passed in which should be
// ignored
struct big_block* descend_expr_ast(ast_node* node, int* tmp_ctr) {
  big_block* ret = new_block;
  struct gen_node_t* n1;
  struct gen_node_t* n2;
  int op;
  quad* final_quad;

  switch (node->type) {
    case (AST_assign):
      struct assign* assign_el = node->obj.a;
      n1 = get_element(assign_el->lvalue, ret, tmp_ctr);
      n2 = get_element(assign_el->rvalue, ret, tmp_ctr);
      if (assign_el->opcode != '=' || assign_el->lvalue->type != IDENT) {
        if (assign_el->opcode == '=') {
          op = Q_STORE;
          final_quad = quad_gen(n1, NULL, n2, op);
          append_quad(ret, final_quad);

        } else {
          // QUAD_CODES has been aligned to make this operation
          // possible
          op = assign_el->opcode - PLUSEQ;
          final_quad = quad_gen(n1, n1, n2, op);
          append_quad(ret, final_quad);
        }
        // Is an IDENT with an opcode '='
      } else {
        final_quad = ret->quad_tail->cur;
        // Replace the final pointer
        final_quad->destination = assign_el->lvalue->obj.l;
      }
      break;

    case (AST_binop):
      struct binop* binop_el = node->obj.b;
      n1 = get_element(binop_el->expr_1, ret, tmp_ctr);
      n2 = get_element(binop_el->expr_2, ret, tmp_ctr);
      struct gen_node_t* tmp = new_tmp(tmp_ctr);
      (*tmp_ctr)++;

      op = ast_quad_op(binop_el->opcode);
      final_quad = quad_gen(tmp, n1, n2, op);

      append_quad(ret, final_quad);
      break;

    case (AST_funct):
      struct funct* funct_el = node->obj.f;

      struct list_node* args = funct_el->args;
      // Might need to make this a pointer or something
      int* counter = 0;
      struct gen_node_t* start = new_const(counter);
      quad* argBegin = quad_gen(NULL, start, NULL, Q_ARGBEGIN);
      append_quad(ret, argBegin);
      while (args->cur != NULL) {
        counter++;
        struct gen_node_t* count_wrap = new_const(counter);
        struct gen_node_t* current_arg =
            get_element(funct_el->args->cur, ret, tmp_ctr);
        quad* arg_quad = quad_gen(NULL, current_arg, count_wrap, Q_ARG);
        append(ret, arg_quad);
      }
      argBegin->src1->data.c = counter;
      n1 = new_tmp(tmp_ctr);
      n2 = new_var(funct_el->name);
      (*tmp_ctr)++;
      quad* func_call_quad = quad_gen(n1, n2, NULL, Q_CALL);
      break;

    case (AST_unop):
      switch (op) {
        n1 = get_element(node->obj.u, ret, tmp_ctr);
        case '*':
          op = Q_LOAD;
          quad* arg_quad = quad_gen(NULL, n1, NULL, Q_LOAD);
          break;
      }
  }

  return ret;
}