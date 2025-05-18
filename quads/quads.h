#include <string.h>
#include "expr.h"

// union generic_node{
//     int 
// }
enum Q_TYPE{
    VAR,
    TMP,
    Q_CONST
};

enum QUAD_CODES{
//Simplest operations
    Q_ADD=0,
    Q_SUB=1,
    Q_MUL,
    Q_DIV,
    Q_MOD,
//Bitwise operations
    Q_SHL,
    Q_SHR,
    Q_BAND,
    Q_BOR,
    Q_XOR,

    Q_LOGNOT,
    Q_LOGOR,
    Q_LOGAND,


    Q_EQUALS,

    Q_GREATER,
    Q_LESS,
    //STORE writes src2 into src1, (which should also be dest)
    Q_STORE,    

    //Pointer dereferencing & UNOPS
    Q_LOAD,
    Q_MOV,

    //Function relatives
    Q_ARGBEGIN,
    Q_ARG,
    Q_CALL,

    //Branches
    Q_BREQ,
    Q_BRNE,

    //Just enough for now: Super basic 
    
};
enum scope{
    LOCAL,
    PARAM,
    GLOBAL,
};

struct tmp {
    int value;
};

struct big_block typedef big_block;

union generic_node{
    char* v; 
    struct tmp* t;
    //Assuming this is just an integer
    int c;
};

struct gen_node_t{
    enum node_type type;
    union generic_node data;
};


struct quad {
    int op;
    struct gen_node_t *destination,*src1,*src2;
} typedef quad;

//Singly linked list of quads or blocks
//If it's a block, the index get's used at print time
union ll_pointer {
    quad* ll_quad;
    big_block* ll_block;
};

struct quad_ll{
    union ll_pointer cur;
    int is_block;
    struct quad_ll* next;
};

struct big_block {
    //Array of pointers
    struct quad_ll* quad_head;
    struct quad_ll* quad_tail;
    int num_el;
    int block_ind;
    char* block_label;
} typedef big_block;


//API
//When control flow logic is detected, generate quads for each of those trees, and then append them to a new big_block
//
// struct big_block* generate_block(struct ast_sym func_base); 


// This operates on a single AST, of undetermined type
// Count temps so they can tick up, clear when a single AST Node is complete 
struct big_block* descend_ast(ast_node* node, int* tmp_ctr, int* block_ctr);
struct big_block* descend_expr_ast(ast_node* node, int* tmp_ctr);
struct big_block* descend_stmt_ast(ast_node* node, int* tmp_ctr, int* block_ctr);
void print_quad_block(big_block* block);