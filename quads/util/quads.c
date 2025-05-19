#include "quads.h"

#include <data.h>
#include <stdio.h>
#include <stdlib.h>

#define new_quad calloc(1, sizeof(struct quad))

int ast_quad_op(int op);

// ll_pointer isn't allocated
big_block* new_block() {
  big_block* new_block = calloc(1, sizeof(struct big_block));
  new_block->block_ind = -1;
  return new_block;
}

void* promotion(struct gen_node_t* r1, struct gen_node_t* r2) {
  if (r2->type == PTR && r1->type == INT) {
    return r2->type;
    // Can add a much more involved cmp_val thing here
  } else if (r1->type == PTR && (r2->type == INT || (r1->type == r2->type))) {
    return r1->type;
  }
  // Error state: only type conversions allowed are int -> ptr
  else {
  }
}

// Creates a tmp node, should always be followed by tmp_ctr
struct gen_node_t* new_tmp(int tmp_ctr, void* type) {
  struct gen_node_t* node = calloc(1, sizeof(struct gen_node_t));
  node->type = TMP;
  struct tmp* data = calloc(1, sizeof(struct tmp));
  data->value = tmp_ctr;
  node->sym_type = type;
  node->data.t = data;
  return node;
}

struct gen_node_t* new_var(char* d) {
  struct gen_node_t* node = calloc(1, sizeof(struct gen_node_t));
  node->type = VAR;
  node->data.v = d;
  return node;
}

struct gen_node_t* new_const(int val) {
  struct gen_node_t* node = calloc(1, sizeof(struct gen_node_t));
  node->type = Q_CONST;
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
// Returns the destination of the last value
struct gen_node_t* append(struct quad_ll* ref, struct quad_ll* new) {
  if (ref->head == NULL) {
    ref->head = new->head;
    ref->tail = new->tail;
  } else {
    ref->tail->next = new->head;
    ref->tail = new->tail;
  }
  free(new);
  return (ref->tail->cur->destination);
}

// Structured like this for more of an API interface
// Underlying data structue changed a lot during development
void append_quad(struct quad_ll* ref, quad* new) {
  struct ll_nodes* ll_node = calloc(1, sizeof(struct ll_nodes));
  if (ref->head == NULL) {
    ref->head = ll_node;
    ref->tail = ll_node;
  } else {
    ref->tail->next = ll_node;
    ref->tail = ll_node;
  }
}

// Each big block gets inserted into the CFG
//  On a branch statement, you do a BREQ or
void create_CFG_tree(big_block* root, ast_node* cond) {}


//funct_decl isn't a real object
//If it existed, it would be the root of the ast, with a list of  
struct CFG* create_funct(struct funct_decl* func, int func_count) {
  struct CFG* cfg_root = calloc(0, sizeof(struct CFG));
  big_block* head = new_block();
  head->func_label = func->func_name;
  head->func_ind = func_count;
  head->block_ind = 0;
  cfg_root->block = head;
  //Using this as a wrapper for whatever statements I get from the AST
  struct list_node* list = func->exprs_or_stmts; 
  while (list->next != NULL) {
    if (list->cur->type < AST_funct) {
      int tmp_ctr = 0;
      descend_expr_ast(list->cur,&(head->quads_list), &tmp_ctr);

    } else if (list->cur->type >= AST_STMT) {

      
    }

    // quads = descend_ast(list->cur,)

  list = list->next;
  }

}

// 3 aspects to a loop
// condition checking, loop body, exit node
//  struct big_block* create_loop(ast_node* loop_head, int* tmp_ctr,
//                                char* func_label, int* block_ctr) {
//    // Stack holds the args in order based on arg_begin.
//    // This should just be another descent, but with a named big block?
//    big_block* ret = new_block();
//    ret->block_ind = *block_ctr;
//    (*block_ctr)++;

//   //Iterate over statements in compound statement associated with loop
//   while(loop_head.next != NULL){
//     big_block* next = descend_ast(loop_head.next, tmp_ctr, block_ctr, ret);
//     loop_head = loop_head.next;
//   }

//   if (next_block->block_ind > 0) {
//     internal_block(ret, next_block);
//   } else {
//     append(ret, next_block);
//   }
//   return ret;
// }

// ast_sym_t* symtab_lookup(node) {
// }

// Descends an expr tree given a node, returning the final element
// If the tree can't be descent further, return the value

struct gen_node_t* get_element(ast_node* node, struct quad_ll* list,
                               int* tmp_ctr) {
  // If this is not a constant or an expression
  if (node == NULL) {
    return NULL;
  }
  if (node->type == AST_ident) {
    // ast_sym_t* symbol = symtab_lookup(node->obj.ident);
    // Replace this with a symtab lookup
    return new_var(node->obj.ident);
  }
  if (node->type == AST_num) {
    return new_const(node->obj.num.val.i);
  } else {
    struct quad_ll* descent = descend_expr_ast(node, list, tmp_ctr);
    return append(list, descent);
  }
}

int parse_assign_q_code(int parser_code) { return parser_code - Q_ADD; };

// Statement that inits a new block (conditions, loops, functs),
struct quad_ll* control_flow(ast_node* node, int* tmp_ctr, int* block_ctr,
                                 big_block* current) {
  // Only supporting while loops
  if (node->type == AST_LOOP) {
    big_block* cond_block = create_loop();
    
  }
}

struct gen_node_t* get_ptr_el(ast_node* node, big_block* ret, int* tmp_ctr) {
  // Outer most node is a unop of type ptr
  // Strip away and return final element
  return get_element(node->obj.u->expr, ret, tmp_ctr);
}

// Stat is -1 for incorrect lval, 0 for ident, 1 for pointer
struct gen_node_t* get_lvalue(ast_node* node, struct quad_ll* list,
                              int* tmp_ctr, int* stat) {
  if (node->type == AST_unop && node->obj.u->opcode == '*') {
    // a pointer dereference
    *stat = 1;
    return get_element(node->obj.u, list, tmp_ctr);
  } else {
    if (node->type != IDENT) {
      stat = -1;
      return 0;
    }
    return get_element(node, list, tmp_ctr);
  }
}

struct quad_ll* descend_expr_ast(ast_node* node, struct quad_ll* list,
                                 int* tmp_ctr) {
  struct gen_node_t* n1;
  struct gen_node_t* n2;
  int op;
  quad* final_quad;

  switch (node->type) {
    case (AST_assign):
      struct assign* assign_el = node->obj.a;
      int stat = 0;
      n1 = get_lvalue(assign_el->lvalue, list, tmp_ctr, &stat);
      n2 = get_element(assign_el->rvalue, list, tmp_ctr);
      // Neither literal nor pointer: just a const basically
      if (stat < 0) {
        printf("Invalid Lvalue: exiting expr\n");
        return list;
      }
      if (assign_el->opcode != '=') {
        int tmp_op = assign_el->opcode - PLUSEQ;
        void* type = promotion(n1, n2);
        struct gen_node_t* tmp = new_tmp(tmp_ctr, type);
        tmp_ctr++;
        quad* tmp_q = quad_gen(tmp, n1, n2, tmp_op);
        append_quad(list, tmp_q);
      }
      // Check if lval is a pointer or not
      if (n1->type == VAR) {
        quad* final_quad = list->tail;
        final_quad->destination = n1;
      } else {
        quad* final_q = quad_gen(NULL, n1, n2, Q_STORE);
        append_quad(list, final_q);
      }
      break;

    case (AST_binop):
      struct binop* binop_el = node->obj.b;
      n1 = get_element(binop_el->expr_1, list, tmp_ctr);
      n2 = get_element(binop_el->expr_2, list, tmp_ctr);
      // Pointer arithmetic
      if (binop_el->opcode == '+' &&
          (n1->type == DATA_PTR || n2->type == DATA_PTR)) {
        int size;
        struct quad_gen_t* ptr = n2;
        struct quad_gen_t* offset = n1;
        // Depending on which is the pointer
        if (n1->type == DATA_PTR) {
          ptr = n1;
          offset = n2;
          // size = sizeof(n1)
        }
        struct gen_node_t* tmp1 = new_tmp(tmp_ctr, ptr->type);
        (*tmp_ctr)++;
        struct gen_node_t* mult = new_const(size);
        quad* inter = quad_gen(tmp1, offset, mult, Q_MUL);
        append_quad(list, inter);
        struct gen_node_t* tmp2 = new_tmp(tmp_ctr, ptr->type);
        (*tmp_ctr)++;
        final_quad = quad_gen(tmp2, ptr, offset, Q_ADD);
        append_quad(list, inter);

      } else if (binop_el->opcode == '-' &&
                 (n1->type == DATA_PTR && n2->type == DATA_PTR)) {
        int size;
        struct gen_node_t* tmp1 = new_tmp(tmp_ctr, n1->type);
        (*tmp_ctr)++;
        struct gen_node_t* mult = new_const(size);

        quad* inter = quad_gen(tmp1, n1, n2, Q_SUB);

        append_quad(list, inter);

        struct gen_node_t* tmp2 = new_tmp(tmp_ctr, n1->type);
        (*tmp_ctr)++;
        final_quad = quad_gen(tmp2, tmp1, size, Q_DIV);
        append_quad(list, inter);
      } else {
        op = ast_quad_op(binop_el->opcode);
        if (op <= Q_LESS && op >= Q_EQUALS) {
          op = L_EQ + op - Q_EQUALS;
          quad* inter_quad = quad_gen(NULL, n1, n2, Q_CMP);
          struct gen_node_t* tmp = new_tmp(*tmp_ctr, n1->type);
          (*tmp_ctr)++;
          final_quad = quad_gen(tmp, NULL, NULL, op);
          append_quad(list, inter_quad);
        } else {
          struct gen_node_t* tmp = new_tmp(*tmp_ctr, n2->type);
          (*tmp_ctr)++;
          final_quad = quad_gen(tmp, n1, n2, op);
          append_quad(list, final_quad);
        }
      }
    case (AST_funct):
      struct funct* funct_el = node->obj.f;

      struct list_node* args = funct_el->args;
      // Might need to make this a pointer or something
      int counter = 0;
      struct gen_node_t* start = new_const(counter);
      quad* argBegin = quad_gen(NULL, start, NULL, Q_ARGBEGIN);
      append_quad(list, argBegin);
      while (args->cur != NULL) {
        counter++;
        struct gen_node_t* count_wrap = new_const(counter);
        struct gen_node_t* current_arg =
            get_element(funct_el->args->cur, list, tmp_ctr);
        quad* arg_quad = quad_gen(NULL, current_arg, count_wrap, Q_ARG);
        append_quad(list, arg_quad);
      }
      argBegin->src1->data.c = counter;
      n1 = new_tmp(*tmp_ctr, n1->type);
      n2 = new_var(funct_el->name->obj.ident);
      (*tmp_ctr)++;
      quad* func_call_quad = quad_gen(n1, n2, NULL, Q_CALL);
      append_quad(list, func_call_quad);
      break;

    case (AST_unop):
      struct unop* unop_el = node->obj.u;
      n1 = get_element(unop_el->expr, list, tmp_ctr);
      // Pointer Dereference
      // Need to be able to get sizeof n1, for all following quads in this
      // scope
      switch (unop_el->opcode) {
        case '*':
          n2 = new_tmp(*tmp_ctr, n1->type);
          (*tmp_ctr)++;
          final_quad = quad_gen(n2, n1, NULL, Q_LOAD);
          break;
        case SIZEOF:
          n2 = new_tmp(*tmp_ctr, n1->type);
          (*tmp_ctr)++;
          // Get size of the first node
          // limiting sizeof to only accept static objs
          //  struct gen_node_t* val = new_const(n1.size);
          final_quad = quad_gen(n2, n1, NULL, Q_MOV);
          break;
          // case PLUSPLUS:
          //   n2 = new_const(1);
          //   final_quad = quad_gen(n1, n1, n2, Q_ADD);
          //   break;
          // case MINUSMINUS:
          //   n2 = new_const(1);
          //   final_quad = quad_gen(n1, n1, n2, Q_SUB);
          //   break;
      }
      append_quad(list, final_quad);
  }

  return list;
}

int ast_quad_op(int op) {
  switch (op) {
    // Arithmetic operators
    case '+':
      return Q_ADD;
    case '-':
      return Q_SUB;
    case '*':
      return Q_MUL;  // Binary multiplication
    case '/':
      return Q_DIV;
    case '%':
      return Q_MOD;
    // Bitwise operators
    // case '&':
    //   return Q_BAND;  // bitwise AND
    // case '|':
    //   return Q_BOR;  // bitwise OR
    // case '^':
    //   return Q_XOR;  // bitwise XOR
    // Relational operators
    case '<':
      return Q_LESS;
    case '>':
      return Q_GREATER;
    // case LOGOR:
    //   return Q_LOGOR;
    // case LOGAND:
    //   return Q_LOGAND;
    // case SHL:
    //   return Q_SHL;
    // case SHR:
    //   return Q_SHR;
    default:
      // If the operator is not in the list above, it's not supported by this
      // function or doesn't have a mapping in the original QUAD_CODES.
      fprintf(stderr,
              "Warning (binop): Operator '%c' has no mapping in original "
              "QUAD_CODES or is not a recognized single-char binary op.\n",
              op);
      // return Q_INVALID;
  }
  return 0;  // Should not happen if all codes are handled
}
