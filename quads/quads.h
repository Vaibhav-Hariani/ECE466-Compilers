#include <string.h>
#include "parser.tab.h"
#include "symbol.h"

// union generic_node{
//     int 
// }
enum node_type{
    VAR,
    TMP
};

enum QUAD_CODES{
    STORE
};

enum scope{
    LOCAL,
    PARAM,
    GLOBAL,
};

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
};

struct quad {
    int op;
    struct gen_node_t *destination,*src1,*src2;
} typedef quad;

struct big_block {
    quad* quads;
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