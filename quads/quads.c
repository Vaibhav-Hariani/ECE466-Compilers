#include "quads.h"

#include <data.h>

#define new_quad calloc(1, sizeof(quad))

big_block* new_block() {
  big_block* new_block = calloc(1, sizeof(big_block));
  new_block->quad_head = calloc(1, sizeof(struct quad_ll));
}

// Creates a tmp node, should always be followed by tmp_ctr
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
//Returns the destination of the last value
struct gen_node_t* append(big_block* ref, big_block* new) {
  ref->quad_tail->next = new->quad_head;
  ref->quad_tail = new->quad_tail;
  ref->num_el += new->num_el;
  // We can safely get rid of new
  // This might be the only memory control in this compiler
  free(new);
  return ref->quad_tail->cur.ll_quad->destination;
}

struct gen_node_t* internal_block(big_block* ref, big_block* new) {
  struct quad_ll* ll_wrapper = calloc(1, sizeof(struct quad_ll));
  ll_wrapper->is_block = 1;
  ll_wrapper->cur.ll_block = new;
  ref -> quad_tail ->next = ll_wrapper;
  ref -> quad_tail = ll_wrapper;
  ref->num_el += new->num_el;
  return ref->quad_tail->cur.ll_quad->destination;
}


// Structured like this for more of an API interface
// Underlying data structue changed a lot during development
void append_quad(big_block* ref, quad* new) {
  struct quad_ll* ll_wrapper = calloc(1, sizeof(struct quad_ll));
  ll_wrapper->cur.ll_quad = new;
  ref->quad_tail->next = ll_wrapper;
  ref->quad_tail = ll_wrapper;
}

// Descends an AST_tree given a node, returning the final element
// If the node can't be descended, return the value
struct gen_node_t* get_element(ast_node* node, big_block* ref, int* tmp_ctr) {
  // If this is not a constant or an expression
  if (node == NULL) {
    return NULL;
  }
  if (node->type == AST_ident) {
    // Replace this with a symtab lookup
    return new_var(node->obj.ident);
  }
  if (node->type == AST_num) {
    return new_const(node->obj.num.val.i);
  }

  big_block* b1 = descend_ast(node, tmp_ctr);
  return append(ref, b1);
}

int parse_assign_q_code(int parser_code) { return parser_code - Q_ADD; };

enum QUAD_CODES ast_quad_op(char op) {
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
    case '&':
      return Q_BAND;  // Binary bitwise AND
    case '|':
      return Q_BOR;  // Binary bitwise OR
    case '^':
      return Q_XOR;  // Binary bitwise XOR

    // Relational operators
    case '<':
      return Q_LESS;
    case '>':
      return Q_GREATER;

    default:
      // If the operator is not in the list above, it's not supported by this
      // function or doesn't have a mapping in the original QUAD_CODES.
      fprintf(stderr,
              "Warning (binop): Operator '%c' has no mapping in original "
              "QUAD_CODES or is not a recognized single-char binary op.\n",
              op);
      // return Q_INVALID;
  }
}

// entrypoint: Pass in a node and a tmp counter
struct big_block* descend_ast(ast_node* node, int* tmp_ctr, int* block_ctr) {
  // if type is a statement, descend AST_STMT
  // Should always be the case on the very first node
  if (node->type == AST_expr) {
    return descend_expr_ast(node, tmp_ctr);
    //For function declarations, pass in the function name: again, should only happen at 
  } else if (node->type == AST_funct) {
    return create_funct(node, tmp_ctr, node.name, block_ctr);    
  }
  big_block* ret =  descend_stmt_ast(node, tmp_ctr, block_ctr);
  *(block_ctr)++;
  return ret;
}

struct big_block* create_func (ast_node* funct_head, int* tmp_ctr, char* func_label, int* block_ctr){
  //Stack holds the args in order based on arg_begin.
  //This should just be another descent, but with a named big block?
  big_block* ret = new_block;
  ret->block_ind = -1;
  ret->block_label = func_label;
  //We only give blocks labels if they're permeanent

  // //Some way to descend the AST from the new head
  // big_block* next_block = descend_ast(funct_head->obj.);
  big_block* next_block;
  if (next_block->block_ind > 0) {
    internal_block(ret,next_block);
  } else {
    append(ret, next_block);
  }
}

struct big_block* descend_stmt_ast(ast_node* node, int* tmp_ctr, int* block_ctr) {
  //If no block name is provided for a function definition or whatnot,
  big_block* ret = new_block;
  ret->block_ind = block_ctr;
  struct gen_node_t* n1;
  struct gen_node_t* n2;
}

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
          // Left side must be a pointer in this case: It'll have a * or
          // something
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
        final_quad = ret->quad_tail->cur.ll_quad;
        // Replace the final pointer with the final quad value
        //This should always evaluate to a quad
        final_quad->destination = assign_el->lvalue->obj.l;
      }
      break;

    case (AST_binop):
      struct binop* binop_el = node->obj.b;
      //FunkyTime!
      n1 = get_element(binop_el->expr_1, ret, tmp_ctr);
      n2 = get_element(binop_el->expr_2, ret, tmp_ctr);
      // TODO:
      // Handling pointer addition: this should be replaced with a function
      // table lookup
      if ((((binop_el->expr_1->type == DATA_PTR) -
            (binop_el->expr_2->type == DATA_PTR)) != 0) &&
          binop_el->opcode == '+') {
        int size;
        // Choose non pointer, create MUL quad
        //  MUL with size
        if (binop_el->expr_1->type == DATA_PTR) {
          // Size == size of expr1;
        } else {
          // Size == size of expr1;
          // Swap n1 and n2 basically
        }
        // struct gen_node_t* size = new_const();
        // quad inter_quad = quad_gen()
        // //get data from n2
        // n2->data.v
      } else if (binop_el->expr_1->type == DATA_PTR &&
                 binop_el->expr_2->type == DATA_PTR &&
                 binop_el->opcode == '-') {
                  //Get size from binop 1
                  int size_of_val; 
                  // size = binop_el->expr_1.

                  //Perform sub op:
                  struct gen_node_t* tmp = new_tmp(tmp_ctr);
                  (*tmp_ctr)++;
                  quad* inter_quad = quad_gen(tmp, n1, n2, Q_SUB);
                  append_quad(ret, inter_quad);
                  struct gen_node_t* size = new_const(size_of_val);
                  //Can re-use tmp from this inter expression                   
                  final_quad = quad_gen(tmp, tmp, size,Q_MUL);
                  append_quad(ret,final_quad);

                } else {
        struct gen_node_t* tmp = new_tmp(tmp_ctr);
        (*tmp_ctr)++;

        op = ast_quad_op(binop_el->opcode);
        final_quad = quad_gen(tmp, n1, n2, op);

        append_quad(ret, final_quad);
      }
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

    //Unops don't evaluate to quads, they should evaluate to 

    case (AST_unop):
        n1 = get_element(node->obj.u,ret,tmp_ctr);
        n2 = new_tmp(tmp_ctr);
        *(tmp_ctr)++;
        int q_op;
        // Pointer Dereference
        // Need to be able to get sizeof n1, for all following quads in this
        // scope
      switch (op) {

        case '*':
          q_op = Q_LOAD;
          final_quad = quad_gen(n2, n1, NULL, op);
          break;
        case SIZEOF:
          op = Q_MOV;
          //Get size of the first node
          //limiting sizeof to only accept static objs
          // struct gen_node_t* val = new_const(n1.size);
          final_quad = quad_gen(n2, n1, NULL, Q_MOV);
      }
      append_quad(ret, final_quad);

  }

  return ret;
}