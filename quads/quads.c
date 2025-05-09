#include "quads.h"

#define new_quad calloc(1,sizeof(quad))
#define new_block calloc(1,sizeof( big_block))

big_block* new_block(){
  calloc(1, sizeof(big_))
}

struct gen_node_t* new_tmp(int tmp_ctr){
    struct gen_node_t* node = calloc(1, sizeof(struct gen_node_t));
    node->type = TMP;
    struct tmp* data = calloc(1, sizeof(struct tmp)); 
    data->value = tmp_ctr;
    node->data.t = data;
    return node;
}
struct gen_node_t* new_var(struct ident* d){
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

big_block* assignment_block(struct assign d, int* tmp_ctr){
    big_block* list_of_blocks[3];
    quad* final_quad;

    big_block* rblock = descend_ast(d.rvalue, tmp_ctr);
    list_of_blocks[0] = rblock;
    int num_blocks = 1;

    switch(d.opcode){
      case '=':
        final_quad = new_quad;
        final_quad->op = rblock->quads[rblock->num_el-1];
    }
      

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


//Appends the quads from new to the end of ref.
//Also allocates space for an extra quad, to save an extra realloc call
struct gen_node_t* append(big_block* ref, big_block* new){
  //Get the node where the output of new is stored
  struct gen_node_t* final = new->quads[new->num_el-1].destination;
  int new_num = ref->num_el + new->num_el + 1;
  quad* new_head = realloc(ref->quads, sizeof(quad) * new_num);
  ref->quads = new_head;
  quad* tail = (ref->quads + ref->num_el);
  //Move all elements in new to ref
  memcpy(new->quads, tail, new->num_el *sizeof(quad));
  //This is currently ahead by one
  //There is an empty space at the end of the array
  ref->num_el = new_num;

  //Free the memory for the old big block and quad array;
  free(new->quads);
  free(new);
  return final;
}

struct big_block* descend_ast(ast_node* node, int* tmp_ctr){
  // struct quad_ref* ret = calloc(1,sizeof(struct quad_ref));
  
  // struct quad_ref* l_block = descend_ast(node->obj.b->expr_1, tmp_ctr);
  // struct  quad_ref* r_block = descend_ast(node->obj.b->expr_2, tmp_ctr++);

  switch(node->type){
    case AST_ident:
      ret->type = SYM;
      ret->element.symbol = new_var(node->obj.ident);
      break;
    case(AST_assign): 

    case(AST_binop):
      struct binop* element = node->obj.b;
      struct gen_node_t* n1 = malloc(sizeof(struct gen_node_t));
      struct gen_node_t* n2 = malloc(sizeof(struct gen_node_t));
      big_block* ret = new_block; 

      if(element->expr_1->type != AST_ident){        
        big_block* b1 = descend_ast(element->expr_1,tmp_ctr);
        n1 = append(ret, b1);
      } else {
        n1 = new_var(element->expr_1->obj.ident);
      } 
      if(element->expr_2->type != AST_ident){        
        big_block* b1 = descend_ast(element->expr_2,tmp_ctr);
        n2 = append(ret, b1);
      } else {
        n2 = new_var(element->expr_2->obj.ident);
      } 


       break;

    default:
      break;
  }
    return ret;     
}