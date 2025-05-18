#include "expr.h"
#include "util/quads.h"
#include "out/quads_out.h"
int main(){
    //Simulating b =  a + 5
    //This is a super duper simplistic implementation: trying anyway?
    ast_node* a = new_ast_ident("a"); 
    ast_node* b = new_ast_ident("b"); 
    TypedNumber n = {0};
    n.type = TYPE_I;
    n.val.i = 5;
    ast_node* node_n = new_ast_num(n);
    ast_node* binop = new_ast_double(AST_binop, a, node_n, '+');
    ast_node* assign = new_ast_double(AST_assign, b, binop, '=');
    int tmp_ctr = 0;
    int block_ctr = 0;
    big_block* block = descend_ast(assign, &tmp_ctr, &block_ctr);
    print_quad_block(block);


}