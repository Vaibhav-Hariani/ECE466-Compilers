#include "quads_out.h"
void print_quad(quad* q) {
  char* code_str = qcode_str_conv(q->op);
  // char* dest = gen_node_str(q->destination);
  // char* src1 = gen_node_str(q->src1);
  // char* src2 = gen_node_str(q->src2);
  //puts dest onto the current output file
  gen_node_str(q->destination);
  //puts dest onto the current output file
  printf("\t %s ", code_str);
  gen_node_str(q->src1);
  //puts dest onto the current output file
  gen_node_str(q->src2);
  // printf("\t %s", code_str);
  // printf("%s \t %s %s %s", dest, code_str, src1, src2);

}

void print_quad_block(big_block* block) {
  if (block->block_label) {
    printf("%s", block->block_label);
  } else {
    printf("BB%d", block->block_ind);
  }
  struct quad_ll* head = block->quad_head;
  while (head != NULL) {
    printf("\n");
    if (head->is_block) {
      print_quad_block(head->cur.ll_block);
    } else {
      print_quad(head->cur.ll_quad);
    }
    head = head->next;
  }
  printf("\n");
}

void gen_node_str(struct gen_node_t* node) {
  if(node == 0){
    return;
  }
  switch (node->type) {
    case VAR:
      printf("%s ",node->data.v);
      break;
    // If I have 4 digit TMP counts something has gone wrong
    case TMP:
      printf("%% T%d ",node->data.t->value);
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
    case Q_SHL:
      return "SHL";
    case Q_SHR:
      return "SHR";
    case Q_BAND:
      return "BAND";
    case Q_BOR:
      return "BOR";
    case Q_XOR:
      return "XOR";
    case Q_LOGNOT:
      return "LOGNOT";
    case Q_LOGOR:
      return "LOGOR";
    case Q_LOGAND:
      return "LOGAND";
    case Q_EQUALS:
      return "EQUALS";
    case Q_GREATER:
      return "GREATER";
    case Q_LESS:
      return "LESS";
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
