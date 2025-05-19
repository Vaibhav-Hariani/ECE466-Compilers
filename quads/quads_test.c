#include "expr.h"
#include "util/quads.h"
#include "out/quads_out.h"
int main(){
    //Simulating b =  a + 5
    //This is a super duper simplistic implementation: trying anyway?
    ast_node* a = new_ast_ident("a", "stdin", 0); 
    ast_node* b = new_ast_ident("b", "stdin", 1); 
    TypedNumber n = {0};
    n.type = TYPE_I;
    n.val.i = 5;
    ast_node* node_n = new_ast_num(n, "stdin",2);
    ast_node* binop = new_ast_double(AST_binop, a, node_n, '*', "stdin", 3);
    ast_node* assign = new_ast_double(AST_assign, b, binop, '=', "stdin", 4);
    int tmp_ctr = 0;
    int block_ctr = 0;
    big_block* block = new_block();
    block->quads_list = *descend_expr_ast(assign, &block->quads_list, &tmp_ctr,0);
    block->func_label = "main";
    block->block_ind = 0;
    print_quad_block(block);



}