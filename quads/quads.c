#include "quads.h"

#define new_quad calloc(1, sizeof(quad))

big_block* new_block() {
  big_block* new_block = calloc(1, sizeof(big_block));
  new_block->quads = malloc(1 * sizeof(quad*));
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
//  Also allocates space for an extra quad, to save an extra realloc call
struct gen_node_t* append(big_block* ref, big_block* new) {
  // Get the node where the output of new is stored
  struct gen_node_t* final = (new->quads[new->num_el])->destination;
  int new_num = ref->num_el + new->num_el;
  quad* new_head = realloc(ref->quads, sizeof(quad) * new_num);
  ref->quads = new_head;
  quad* tail = (ref->quads + ref->num_el);
  // Move all elements in new to ref
  memcpy(new->quads, tail, new->num_el * sizeof(quad));

  ref->num_el = new_num;
  // Free the memory for the old big block and quad array;
  free(new->quads);
  free(new);
  return final;
}

struct gen_node_t* get_element(ast_node* node, big_block* ref, int* tmp_ctr) {
  if (node->type != AST_ident) {
    big_block* b1 = descend_ast(node, tmp_ctr);
    return append(ref, b1);
  }
  return new_var(node->obj.ident);
}

struct big_block* descend_expr_ast(ast_node* node, int* tmp_ctr) {
  big_block* ret = new_block;
  struct gen_node_t* n1;
  struct gen_node_t* n2;
  int op;

  switch (node->type) {
    case (AST_assign):
      struct assign* assign_el = node->obj.a;
      n1 = get_element(assign_el->lvalue, ret, tmp_ctr);
      n2 = get_element(assign_el->rvalue, ret, tmp_ctr);

      // Assuming that, if lvalue is not an ident, we are getting a pointer
      // value. Then, a store operation will be necessary.
      // Otherwise, dealing with a var, and can just replace the
      // last stored value of n2

      //If the last operation is not a '='
      if(assign_el->opcode != '=' || assign_el->lvalue->type != IDENT){
        if(assign_el->opcode == '='){
          op = Q_STORE;
        } else {
          //QUAD_CODES has been aligned to make this operation possible 
          op = assign_el->opcode - PLUSEQ;
        }
        quad* final_quad = quad_gen(n1, n1, n2, op);

      }
      if (assign_el->lvalue->type != IDENT) {
        op = Q_STORE;
        // Special 2 addr opcode
        quad* final_quad = quad_gen(0, n1, n2, op);
        //This shouldn't work
        ret->quads[ret->num_el - 1] = final_quad;
      } else {
        quad* last_quad = ret->quads[ret->num_el - 1];
        last_quad->destination = n1;
      }

      break;
    case (AST_binop):
      struct binop* binop_el = node->obj.b;
      n1 = get_element(binop_el->expr_1, ret, tmp_ctr);
      n2 = get_element(binop_el->expr_2, ret, tmp_ctr);
      struct gen_node_t* tmp = new_tmp(tmp_ctr);
      (*tmp_ctr)++;
      quad* final_quad = quad_gen(tmp, n1, n2, binop_el->opcode);
      // TODO: This is broken, need to fix so that a final_quad can be appended
      ret->quads[ret->num_el - 1] = final_quad;
      break;

    default:
      break;
  }

  return ret;
}