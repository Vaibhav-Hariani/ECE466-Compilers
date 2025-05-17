#include "quads.h"

#define new_quad calloc(1,sizeof(quad))
#define new_block calloc(1,sizeof( big_block))


big_block* generate_blocks(struct ast_sym func_base) {}

// This only handles expressions, converts them to
quad* generate_quads(struct ast_node node) {
  // Descend the ast in a depth first search manner
  // assuming node has left
  int tmp = 0;
  descend_ast(node, 0);
}

struct gen_node_t* new_t(int tmp_ctr){
    struct gen_node_t* node = calloc(1, sizeof(struct gen_node_t));
    node->type = TMP;
    struct tmp* data = calloc(1, sizeof(struct tmp)); 
    data->value = tmp_ctr;
    node->data.t = data;
    return node;
}
struct gen_node_t* new_v(struct ident* d){
  struct gen_node_t* node = calloc(1, sizeof(struct gen_node_t));
  node->type= VAR;
  node->data.t = d;
  return node;
}

quad* quad_gen(struct gen_node_t* dest, struct gen_node_t* src1,struct gen_node_t* src2, int quad_code){
  quad* quad = new_quad;
  quad->destination = dest;
  quad ->op = quad_code;
  quad -> src1 = src1;
  quad -> src2 = src2;
  return quad;
}

void append_quad(quad* q1, big_block* block){
  block->num_el++;
  quad* new_head = realloc(block->quads, sizeof(quad) *block->num_el);
  block->quads = new_head;
}

//Assuming a simple assignment
// Proof of concept

big_block* assignment_block(struct assign d, int* tmp_ctr){
    big_block* list_of_blocks[3];
    quad* final_quad;

    big_block* rblock = descend_ast(d.rvalue, tmp_ctr);
    list_of_blocks[0] = rblock;
    int num_blocks = 1;

    if(d.lvalue->type == IDENT){      
      // final_quad->destination = d.lvalue->obj.ident;
      final_quad = (rblock->quads) + rblock->num_el - 1;
      if(final_quad->destination->type==TMP){
        final_quad->destination = d.lvalue->obj.ident;
      } else {
        //Something really weird has happened here
        perror("Error in assignment block creation.\n");
      }
      //Somewhere here, some compound assignment stuff could happen
      return rblock;
    //Some form of weird dereferncing is happening, we need to simplify
    } else {
      big_block* lval = descend_ast(d.lvalue, tmp_ctr);
      num_blocks = 2;
      list_of_blocks[0] = lval;
      final_quad = new_quad;
      final_quad->destination = (lval->quads + lval->num_el - 1)->destination;
      final_quad->op = STORE;      
      big_block* final_block = combine_big_blocks(list_of_blocks,2);
      append_quad(final_block, final_quad);
      return final_block;
    }
}

big_block* binop_block(struct binop b, int* tmp_ctr){
  big_block* list_of_blocks[2];
  int num_blocks = 0;
  big_block* block1;

  if(b.expr_1 != IDENT){
    block1 = descend_ast(b.expr_1,tmp_ctr);
    list_of_blocks[num_blocks] = block1;
    num_blocks++;

  } else {
    block1 = new_block;
    descend_ast(b.expr_1,tmp_ctr);
    list_of_blocks[num_blocks] = block;
    num_blocks++;

  }
  if(b.expr_2 != IDENT){
    big_block* block = descend_ast(b.expr_1,tmp_ctr);
    list_of_blocks[num_blocks] = block;
    num_blocks++;

  }

  big_block* rblock = descend_ast(d.rvalue, tmp_ctr);
  list_of_blocks[0] = rblock;
  int num_blocks = 1;

  quad* final_quad = new_quad;
  final_quad->destination = new_t(tmp_ctr);
  tmp_ctr++;
}



 big_block* binop(struct assign d, int* tmp_ctr){}

 big_block* descend_ast(ast_node* node, int* tmp_ctr){
  switch (node->type){
    case 'x':
    break;
  }  

    struct big_block* final = calloc(1, sizeof(struct big_block));
 }

// Descends the AST given a specific node
 big_block* descend_ast(ast_node node, int tmp_counter) {
  int num_elements = 0;
  struct big_block dest = {0};
  struct big_block src1, src2 = {0};
  struct ast_sym ast_children[3] = {0};
  int opcode = 0;
  switch (node.data->type) {
    // TODO: Change this to something rational

    // Ternops should have their own routine for parsing
    //  as they should break up until multiple quads
    case AST_ternop:
      num_elements = 3;
      ast_children[0] = node.ternop.expr_1;
      ast_children[1] = node.ternop.expr_2;
      ast_children[2] = node.ternop.expr_2;
      break;
    case AST_binop:
      num_elements = 2;
      ast_children[0] = node.binop.expr_1;
      ast_children[1] = node.binop.expr_2;
      opcode = node.opcode;
      break;
    case AST_unop:
      num_elements = 1;
      ast_children[0] = node.binop.expr_1;
      ast_children[1] = node.binop.expr_2;
      opcode = node.opcode;
      break;
    case AST_assign:
      num_elements = 2;
      ast_children[0] = node.binop.expr_1;
      ast_children[1] = node.binop.expr_1;
      break;
      /*
        ... for all ast types:
        Some behavior that should be handled
        All functs should have a big block label already, on our pass through the
        scopes before this quad exec happens we should store params call function
        save return type to a tmp
        This and ternop is the only "routine" that exists within an expression      
      */
    case AST_funct:
      break;
    // This element should never be an ident, this is an error state
    case AST_ident:
      break;
  }
}



 big_block* combine_big_blocks( big_block** blocks,
                                     int num_blocks) {
  int new_num = 0;
  for (int i = 0; i < num_blocks; i++) {
    new_num += blocks[i]->num_el;
  }
   quad* new_head = realloc(blocks[0]->quads, sizeof(quad) * new_num);
  blocks[0]->quads = new_head;
   quad* head = new_head + blocks[0]->num_el;
  for (int i = 1; i < num_blocks; i++) {
    int num_move = blocks[i]->num_el;
    quad* loc = blocks[i]->quads;
    // Copy all bytes from this array to new head
    memmove(head, loc, sizeof(quad) * num_move);
    head = head + num_move;
    free(blocks[i]);
  }
  (*blocks)->num_el = new_num;
  return blocks;
}