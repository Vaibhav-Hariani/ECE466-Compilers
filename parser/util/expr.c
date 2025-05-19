#include "expr.h"

#define new_ast_node calloc(1, sizeof(struct ast_node))

ast_node* new_ast_ident(char* c, char *filename, int line) {
  ast_node* node = new_ast_node;
  node->type = AST_ident;
  node->obj.ident = c;
  node->filename = filename;
  node->line = line;
  return node;
}

ast_node* new_ast_num(TypedNumber n, char *filename, int line) {
  ast_node* node = new_ast_node;
  node->type = AST_num;
  node->obj.num = n;
  node->filename = filename;
  node->line = line;
  return node;
}

ast_node* new_ast_charlit(char c, char *filename, int line) {
  ast_node* node = new_ast_node;
  node->type = AST_num;
  node->obj.charlit = c;
  node->filename = filename;
  node->line = line;
  return node;
}

ast_node* new_ast_string(SizedString s, char *filename, int line) {
  ast_node* node = new_ast_node;
  node->type = AST_string;
  node->obj.str = s;
  node->filename = filename;
  node->line = line;
  return node;
}

struct list_node* new_list_node(ast_node* head){
  struct list_node* l = calloc(1,sizeof(struct list_node));
  l->cur = head;
  l->next = 0;
  return l;
}

ast_node* new_ast_list(ast_node* head){
  struct list_node* l = new_list_node(head);
  ast_node* node = new_ast_node;
  node->type=AST_list;
  node->obj.l = l;
  return node;
}

ast_node* append_ast_list(ast_node* root, ast_node* new){
  if(root->type != AST_list){
    // yyerror("List was not a list");
    exit(1);
  }
  struct list_node* head = root->obj.l;
  
  if(head->cur == 0){
    head->cur = new;
  }
  struct list_node* tail = new_list_node(new);
  while(head->next != 0){
    head = head->next;
  }
  head->next = tail;
  return root;
}

ast_node* ast_array_exp(ast_node* expr1, ast_node* expr2, char *filename, int line){
  ast_node* inner_expr = new_ast_double(AST_binop, expr1, expr2, '+', strdup(filename), line);
  return new_ast_single(inner_expr, '*', PREFIX, filename, line);
}


// Given that both both children of a binop are numbers or char literals, this should convert them into a new numlit/charlit.

ast_node* new_ast_double(int type, ast_node* expr1, ast_node* expr2, int op, char *filename, int line) {
  ast_node* node = new_ast_node;
  node->filename = filename;
  node->line = line;
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
    case AST_assign:;
      struct assign* obj = calloc(1, sizeof(struct assign));
      obj->lvalue = expr1;
      obj->rvalue = expr2;
      obj->opcode = op;

      node->type = AST_assign;
      node->obj.a = obj;

      // // hackier lvalue handling
      // if (expr1->is_lval != 1) {
      //   yyerror("Expr1 in assignment not an lvalue");
      //   exit(1);
      // }
      break;
    
    case AST_funct:;
      struct funct* function = calloc(1, sizeof(struct funct));
      function->name= expr1;
      if(expr2->type !=AST_list) {
        // yyerror("Function Call args are not of type list");
        exit(1);
      }
      function->args = expr2->obj.l;
      node->type = AST_funct;
      node->obj.f = function;
      break;

    default:
      // yyerror("Not a real binop");
      exit(1);
      break;
  }
  return node;
}

ast_node* new_ast_ternop(int type, ast_node* expr1, ast_node* expr2,
                         ast_node* expr3, char *filename, int line) {
  ast_node* node = new_ast_node;
  node->type = AST_ternop;
  struct ternop* obj = calloc(1, sizeof(struct ternop));
  obj->expr_1 = expr1;
  obj->expr_2 = expr2;
  obj->expr_3 = expr3;
  node->obj.t = obj;
  node->filename = filename;
  node->line = line;
  return node;
}

ast_node* new_ast_single(ast_node* expr, int op, int dir, char *filename, int line) {
  ast_node* node = new_ast_node;
  node->type = AST_unop;
  struct unop* obj = calloc(1, sizeof(struct unop));
  obj->expr = expr;
  obj->opcode = op;
  obj->sequence = dir;
  node->obj.u = obj;
  // Allowing lvalue status to "trickle up" through unops
  // This should hopefully make my life easier going forward
  node->is_lval = expr->is_lval;
  node->filename = filename;
  node->line = line;
  return node;
}