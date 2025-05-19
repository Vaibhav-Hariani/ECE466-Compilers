#include "quads_out.h"
void print_quad(quad* q) {
  char* code_str = qcode_str_conv(q->op);
  // char* dest = gen_node_str(q->destination);
  // char* src1 = gen_node_str(q->src1);
  // char* src2 = gen_node_str(q->src2);
  //puts dest onto the current output file
  printf("  ");

  gen_node_str(q->destination);
  //puts dest onto the current output file
  printf("= %s ", code_str);
  gen_node_str(q->src1);

  //puts dest onto the current output file
  gen_node_str(q->src2);
  // printf("\t %s", code_str);
  // printf("%s \t %s %s %s", dest, code_str, src1, src2);

}

void print_quad_block(big_block* block) {
  if (block->func_label) {
    printf("%s: \n", block->func_label);
  }
  printf(" BB%d:", block->block_ind);
  struct ll_nodes* head = block->quads_list.head;
  while (head != NULL) {
    printf("\n");
    print_quad(head->cur);
    head = head->next;
    }
  }



void gen_node_str(struct gen_node_t* node) {
  if(node == 0){
    return;
  }
  switch (node->type) {
    case VAR:
      if(node->symbol != 0){
        printf("%s {%s} ",node->data.v, sco_type_to_str(node->symbol->sco_type));
      }
      else {
        printf("%s ",node->data.v);
      }
      break;
    // If I have 4 digit TMP counts something has gone wrong
    case TMP:
      printf("%%T%d ",node->data.t->value);
      break;
    case Q_CONST:
      printf("%d ", node->data.c);
      break;
  }
}


char* qcode_str_conv(int code) {
  switch (code) {
    case Q_ADD:
      return "ADD";
    case Q_SUB:
      return "SUB";
    case Q_MUL:
      return "MUL";
    case Q_DIV:
      return "DIV";
    case Q_MOD:
      return "MOD";
    case L_EQ:
      return "EQ";
    case L_GR:
      return "GR";
    case L_GEQ:
      return "GEQ";
    case L_LEQ:
      return "LEQ";
    case L_LT:
      return "LT";
    case Q_CMP:
      return "CMP";
    case Q_STORE:
      return "STORE";
    case Q_LOAD:
      return "LOAD";
    case Q_ARGBEGIN:
      return "ARGBEGIN";
    case Q_ARG:
      return "ARG";
    case Q_CALL:
      return "CALL";
    case Q_BREQ:
      return "BREQ";
    case Q_BRNE:
      return "BRNE";

  }
  return "UNKNOWN_Q_CODE";  // Should not happen if all codes are handled
}

char* sco_type_to_str(char type) {
    switch (type) {
        case SCO_NONE:
            return "NONE";
        case SCO_FILE:
            return "FILE";
        case SCO_FUNC:
            return "FUNCT";
        case SCO_BLOCK:
            return "BLOCK";
        case SCO_PROTO:
            return "PROTOTYPE";
        case SCO_STRUNIO:
            return "ST_UNION";
        case SCO_ENU:
            return "ENUM";
        default:
            return "Unknown Scope";
    }
  }