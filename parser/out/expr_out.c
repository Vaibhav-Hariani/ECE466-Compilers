#include "expr_out.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../parser.tab.h"

// Currently, string is the last token
// Returns an array of pointers, that allows us to quickly find & fix tokens
char** token_labels() {
  char** tokens = calloc(STRING + 1, sizeof(tokens));
  /*
  // Unused, commented out
  tokens[YYEOF] ="YYEOF";
  tokens[YYerror] ="YYerror";
  tokens[YYUNDEF] ="YYUNDEF";
  */
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

void print_spaces(int num_spaces) {
    char tab_arr[num_spaces + 1];
    memset(tab_arr, ' ', num_spaces);
    tab_arr[num_spaces] = '\0';
    printf("%s", tab_arr);
}

// Helper function to print an int
void print_num(TypedNumber num) {
  if (num.type <= TYPE_ULLI) {
    printf("NUMLIT INT: %lld \n", num.val.i);
  } else {
    printf("NUMLIT FLOAT: %Lg \n", num.val.f);
  }
};

void opcode_print(int opcode, char** tokens, char* pre_msg, char* post_msg){
  printf("%s", pre_msg);
  if (opcode <= 255) {
    printf("%c %s \n", opcode, post_msg);
  } else {
    printf("%s %s \n", tokens[opcode], post_msg);
  }

}

ast_node* print_ast(ast_node* expr, int num_spaces) {
  char** tokens = token_labels();
  print_recurse(expr, num_spaces, tokens);
  return expr;
}


void print_recurse(ast_node* expr, int num_spaces, char** tokens) {
  char* c;

  int opcode;
  switch (expr->type) {
    case AST_binop:;

      struct binop* b = expr->obj.b;
      opcode = b->opcode;

      print_spaces(num_spaces);
      opcode_print(opcode, tokens, "BINARY OP ", "");

      print_recurse(b->expr_1, num_spaces + 1, tokens);

      print_recurse(b->expr_2, num_spaces + 1, tokens);
      /* code */
      break;

    case AST_ternop:;
      struct ternop* t = expr->obj.t;

      print_spaces(num_spaces);
      printf("TERNARY: \n");

      print_spaces(num_spaces + 1);
      printf("EXPR 1:\n");
      print_recurse(t->expr_1, num_spaces + 2, tokens);

      print_spaces(num_spaces + 1);
      printf("EXPR 2:\n");
      print_recurse(t->expr_2, num_spaces + 2, tokens);

      print_spaces(num_spaces + 1);
      printf("EXPR 3:\n");
      print_recurse(t->expr_3, num_spaces + 2, tokens);

      break;

    case AST_unop:;
      struct unop* u = expr->obj.u;
      char* pre_post = (u->sequence == PREFIX) ? "PREFIX" : "POSTFIX";
      opcode = u->opcode;

      print_spaces(num_spaces);
      opcode_print(opcode, tokens, "UNARY OP ", pre_post);

      print_recurse(u->expr, num_spaces + 1, tokens);

      break;

    case AST_assign:;
      struct assign* a = expr->obj.a;
      opcode = a->opcode;

      print_spaces(num_spaces);
      opcode_print(opcode, tokens, "ASSIGNMENT ", "");

      print_spaces(num_spaces + 1);
      printf("LVAL:\n");
      print_recurse(a->lvalue, num_spaces + 2, tokens);

      print_spaces(num_spaces + 1);
      printf("RVAL:\n");
      print_recurse(a->rvalue, num_spaces + 2, tokens);
      break;
    
    case AST_funct:;
      struct funct* f = expr->obj.f;
      print_spaces(num_spaces);
      printf("FUNCTION CALL:\n");

      print_spaces(num_spaces + 1);
      printf("FUNCTION:\n");
      print_recurse(f->name, num_spaces + 2, tokens);

      print_spaces(num_spaces + 1);
      printf("ARGS:\n");

      struct list_node args = *(f->args);
      while(args.cur != 0) {
        print_recurse(args.cur,num_spaces + 2, tokens);
        if(args.next != 0){
          args=*(args.next);
        } else{
          break;
        }
      }
      break;


    case AST_ident:
      c = expr->obj.ident;
      print_spaces(num_spaces);
      printf("IDENT: %s\n", c);
      break;

    case AST_string:
      SizedString str = expr->obj.str;
      print_spaces(num_spaces);
      printf("STRINGLIT: ");
      for(int i = 0; i < str.size; i++){
        putc(str.li[i], stdout);
      }
      putc('\n', stdout);
      break;

    case AST_charlit:;
      char lit = expr->obj.charlit;
      print_spaces(num_spaces);
      printf("CHARLIT: %c\n", lit);
      break;

    case AST_num:;
      TypedNumber n = expr->obj.num;
      print_spaces(num_spaces);
      print_num(n);
      break;

    default:
      fprintf(stderr, "Unkown Expression Type: Failed\n");
      exit(1);
      break;
  }
}