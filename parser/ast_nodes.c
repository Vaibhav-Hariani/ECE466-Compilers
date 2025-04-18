#include "ast_nodes.h"

#include <stdlib.h>

#include "parser.tab.h"

#define new_ast_node calloc(1, sizeof(struct ast_node))

ast_node_t* new_ast_ident(char* c) {
  ast_node_t* node = new_ast_node;
  node->type = AST_ident;
  node->obj.ident = c;
  return node;
}

ast_node_t* new_ast_num(TypedNumber n) {
  ast_node_t* node = new_ast_node;
  node->type = AST_num;
  node->obj.num = n;
  return node;
}

ast_node_t* new_ast_charlit(char c) {
  ast_node_t* node = new_ast_node;
  node->type = AST_charlit;
  node->obj.charlit = c;
  return node;
}

ast_node_t* new_ast_string(SizedString s) {
  ast_node_t* node = new_ast_node;
  node->type = AST_string;
  node->obj.str = s;
  return node;
}

ast_node_t* new_ast_lvalue(ast_node_t* expr) {
  expr->is_lval = 1;
  return expr;
}


// Given that both both children of a binop are numbers or char literals, this should convert them into a new numlit/charlit.

ast_node_t* new_ast_binop(int type, ast_node_t* expr1, ast_node_t* expr2, int op) {
  ast_node_t* node = new_ast_node;
  struct binop* bin = calloc(1, sizeof(struct binop));

  switch (type) {
    case AST_binop:
      // if(expr1->type >= AST_charlit && expr2->type >= AST_charlit) {
      // These lines should handle num literal parsing
      // }
      
      node->type = AST_binop;
      bin->expr_1 = expr1;
      bin->expr_2 = expr2;
      bin->opcode = op;
      node->obj.b = bin;
      break;
    case AST_assign:
      struct assign* obj = calloc(1, sizeof(struct assign));
      obj->lvalue = expr1;
      obj->rvalue = expr2;
      obj->opcode = op;

      node->type = AST_assign;
      node->obj.a = obj;

      // hackier lvalue handling
      if (expr1->is_lval != 1) {
        yyerror("Expr1 in assignment not an lvalue");
        exit(1);
      }
      break;

    case AST_special:
      struct special* spec = calloc(1, sizeof(struct special));
      spec->expr_1 = expr1;
      spec->expr_2 = expr2;
      node->type = AST_special;
      spec->opcode = op;
      node->obj.s = spec;
      break;

    default:
      yyerror("Not a real binop");
      exit(1);
      break;
  }
  return node;
}

ast_node_t* new_ast_ternop(int type, ast_node_t* expr1, ast_node_t* expr2,
                         ast_node_t* expr3) {
  ast_node_t* node = new_ast_node;
  node->type = AST_ternop;
  struct ternop* obj = calloc(1, sizeof(struct ternop));
  obj->expr_1 = expr1;
  obj->expr_2 = expr2;
  obj->expr_3 = expr3;
  node->obj.t = obj;
  return node;
}

ast_node_t* new_ast_unop(ast_node_t* expr, int op, int dir) {
  ast_node_t* node = new_ast_node;
  node->type = AST_unop;
  struct unop* obj = calloc(1, sizeof(struct unop));
  obj->expr = expr;
  obj->opcode = op;
  obj->sequence = dir;
  node->obj.u = obj;
  // Allowing lvalue status to "trickle up" through unops
  // This should hopefully make my life easier going forward
  node->is_lval = expr->is_lval;
  return node;
}
