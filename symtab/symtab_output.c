#include "symtab_output.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.tab.h"

// Currently, string is the last token
// Returns an array of pointers, that allows us to quickly find & fix tokens
char** token_labels() {
  char** tokens = calloc(STRING + 1, sizeof(tokens));
  tokens[YYEOF] ="YYEOF";
  tokens[YYerror] ="YYerror";
  tokens[YYUNDEF] ="YYUNDEF";
  tokens[TOKEOF] ="TOKEOF";
  tokens[ELLIPSIS] ="...";
  tokens[POUNDPOUND] ="##";
  tokens[AUTO] ="AUTO";
  tokens[BREAK] ="BREAK";
  tokens[CASE] ="CASE";
  tokens[CHAR] ="CHAR";
  tokens[CONST] ="CONST";
  tokens[CONTINUE] ="CONTINUE";
  tokens[DEFAULT] ="DEFAULT";
  tokens[DO] ="DO";
  tokens[DOUBLE] ="DOUBLE";
  tokens[ELSE] ="ELSE";
  tokens[ENUM] ="ENUM";
  tokens[EXTERN] ="EXTERN";
  tokens[FLOAT] ="FLOAT";
  tokens[FOR] ="FOR";
  tokens[GOTO] ="GOTO";
  tokens[IF] ="IF";
  tokens[INLINE] ="INLINE";
  tokens[INT] ="INT";
  tokens[LONG] ="LONG";
  tokens[REGISTER] ="REGISTER";
  tokens[RESTRICT] ="RESTRICT";
  tokens[RETURN] ="RETURN";
  tokens[SHORT] ="SHORT";
  tokens[SIGNED] ="SIGNED";
  tokens[STATIC] ="STATIC";
  tokens[STRUCT] ="STRUCT";
  tokens[SWITCH] ="SWITCH";
  tokens[TYPEDEF] ="TYPEDEF";
  tokens[UNION] ="UNION";
  tokens[UNSIGNED] ="UNSIGNED";
  tokens[VOID] ="VOID";
  tokens[VOLATILE] ="VOLATILE";
  tokens[WHILE] ="WHILE";
  tokens[BOOL] ="BOOL";
  tokens[COMPLEX] ="COMPLEX";
  tokens[IMAGINARY] ="IMAGINARY";
  tokens[PLUSPLUS] ="++";
  tokens[MINUSMINUS] ="--";
  tokens[PLUSEQ] ="+=";
  tokens[MINUSEQ] ="-=";
  tokens[DIVEQ] ="/=";
  tokens[TIMESEQ] ="*=";
  tokens[MODEQ] ="%%=";
  tokens[SHLEQ] ="<<=";
  tokens[SHREQ] =">>=";
  tokens[ANDEQ] ="&=";
  tokens[OREQ] ="|=";
  tokens[XOREQ] ="^=";
  tokens[LOGAND] ="&&";
  tokens[LOGOR] ="||";
  tokens[EQEQ] ="==";
  tokens[NOTEQ] ="!=";
  tokens[GTEQ] =">=";
  tokens[LTEQ] ="<=";
  tokens[SHL] ="<<";
  tokens[SHR] =">>";
  tokens[SIZEOF] ="SIZEOF";
  tokens[PREFIX] ="PREFIX";
  tokens[POSTFIX] ="POSTFIX";
  tokens[INDSEL] ="->";
  tokens[IDENT] ="IDENT";
  tokens[CHARLIT] ="CHARLIT";
  tokens[NUM] ="NUM";
  tokens[STRING] ="STRING";
  return tokens;
}

ast_node_t* print_ast(ast_node_t* expr) {
  char** tokens = token_labels();
  print_recurse(expr, 0, tokens);
  return expr;
}

// Helper function to print an int
void print_num(TypedNumber num) {
  if (num.type <= TYPE_ULLI) {
    fprintf(stderr, "NUMLIT: %lld \n", num.val.i);
  } else {
    fprintf(stderr, "NUMLIT: %Lg \n", num.val.f);
  }
};

// Helper function to print the special objects
void print_special(struct special* s, int num_tabs, char** tokens) {
  num_tabs++;
  char* str1;
  char* str2;
  char* str3;
  char tab_arr[num_tabs + 1];
  memset(tab_arr, '\t', num_tabs);
  tab_arr[num_tabs] = '\0';

  switch (s->opcode) {
    case (']'):
      str1 = "ARRAY INDEX:";
      str2 = "OBJECT:";
      str3 = "INDEX:";
      break;
    case (')'):
      str1 = "FUNCTION CALL:";
      str2 = "FUNCTION:";
      str3 = "PARAMS:";
      break;
    case ('.'):
      str1 = "MEMBER ACCESS";
      str2 = "PARENT:";
      str3 = "MEMBER:";
      break;
    case (INDSEL):
      str1 = "MEMBER DEREFRENCE";
      str2 = "PARENT:";
      str3 = "MEMBER:";
      break;
  }
  fprintf(stderr, "%s\n", str1);
  fprintf(stderr, "%s %s\n", tab_arr, str2);
  print_recurse(s->expr_1, num_tabs + 1, tokens);
  if (s->expr_2 != NULL) {
    fprintf(stderr, "%s %s\n", tab_arr, str3);
    print_recurse(s->expr_2, num_tabs + 1, tokens);
  }
}

// As parser becomes larger and larger, this will need to be converted to a more
// modular approach I'm thinking of using the enums as function pointers to
// specific print functions. However, this bloated mess works for this
// assignment that is currently much later than I (or you) would like
void print_recurse(ast_node_t* expr, int num_tabs, char** tokens) {
  char tab_arr[num_tabs + 1];
  char* c;

  memset(tab_arr, '\t', num_tabs);
  tab_arr[num_tabs] = '\0';
  fprintf(stderr, "%s", tab_arr);

  int opcode;
  switch (expr->type) {
    case AST_binop:
      struct binop* b = expr->obj.b;
      opcode = b->opcode;

      if (opcode <= 255) {
        fprintf(stderr, "BINARY OP %c \n", opcode);
      } else {
        fprintf(stderr, "BINARY OP %s \n", tokens[opcode]);
      }
      print_recurse(b->expr_1, num_tabs + 1, tokens);
      print_recurse(b->expr_2, num_tabs + 1, tokens);
      /* code */
      break;

    case AST_ternop:
      struct ternop* t = expr->obj.t;
      fprintf(stderr, "TERNARY: \n");
      fprintf(stderr, "%s EXPR 1: \n", tab_arr);
      print_recurse(t->expr_1, num_tabs + 1, tokens);
      fprintf(stderr, "%s EXPR 2: \n", tab_arr);
      print_recurse(t->expr_2, num_tabs + 1, tokens);
      fprintf(stderr, "%s EXPR 3: \n", tab_arr);
      print_recurse(t->expr_3, num_tabs + 1, tokens);
      break;

    case AST_unop:
      struct unop* u = expr->obj.u;
      char* pre_post = (u->sequence == PREFIX) ? "PREFIX" : "POSTFIX";
      opcode = u->opcode;
      if (opcode <= 255) {
        fprintf(stderr, "UNARY OP %c %s \n", opcode, pre_post);
      } else {
        fprintf(stderr, "UNARY OP %s %s \n", tokens[u->opcode], pre_post);
      }
      print_recurse(u->expr, num_tabs + 1, tokens);
      break;

    case AST_assign:
      struct assign* a = expr->obj.a;
      opcode = a->opcode;
      if (opcode < 255) {
        fprintf(stderr, "ASSIGNMENT \' %c \', \n", opcode);
      } else {
        fprintf(stderr, "ASSIGNMENT \' %s \', \n", tokens[opcode]);
      }
      fprintf(stderr, "%s LVAL: \n", tab_arr);
      print_recurse(a->lvalue, num_tabs + 1, tokens);
      fprintf(stderr, "%s RVAL: \n", tab_arr);
      print_recurse(a->rvalue, num_tabs + 1, tokens);
      break;

    case AST_ident:
      c = expr->obj.ident;
      fprintf(stderr, "IDENT: %s \n", c);
      break;

    case AST_string:
      c = expr->obj.str.li;
      fprintf(stderr, "STRINGLIT: %s \n", c);
      break;

    case AST_charlit:
      char lit = expr->obj.charlit;
      fprintf(stderr, "CHARLIT: %c: \n", lit);
      break;

    case AST_num:
      TypedNumber n = expr->obj.num;
      print_num(n);
      break;

    case AST_special:
      struct special* s = expr->obj.s;
      print_special(s, num_tabs, tokens);
      break;

    default:
      fprintf(stderr, "Unkown Expression Type: Failed \n");
      exit(1);
      break;
  }
}
