#include <string.h>
#include "parser.tab.h"
#include "symbol.h"

// union generic_node{
//     int 
// }
enum node_type{
    VAR,
    TMP,
    CONST
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

    //Pointer dereferencing
    Q_LOAD,

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

//This should be replaced with something from
//The AST
struct ident {
    char* label;
    enum scope scope;
};

struct tmp {
    int value;
};

struct gen_node_t{
    enum node_type type;
    union generic_node data;
};

union generic_node{
    struct ident* v; 
    struct tmp* t;
    //Assuming this is just an integer
    int c;
};

struct quad {
    int op;
    struct gen_node_t *destination,*src1,*src2;
} typedef quad;

//Singly linked list of quads
struct quad_ll{
    quad* cur;
    struct quad_ll* next;
};

struct big_block {
    //Array of pointers
    struct quad_ll* quad_head;
    struct quad_ll* quad_tail;

    int num_el;
    int block_ind;
} typedef big_block;


//API
//When control flow logic is detected, generate quads for each of those trees, and then append them to a new big_block
//
struct big_block* generate_blocks(struct ast_sym func_base); 


// This operates on a single AST. No control logic
// Count temps so they can tick up, clear when a single AST Node is complete 
struct quad* generate_quads(struct ast_node node);