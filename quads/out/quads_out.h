#include "../util/quads.h"
#include <stdlib.h>
#include <stdio.h>
char* qcode_str_conv(int code);
void gen_node_str(struct gen_node_t* node);
void print_quad(quad* q);
void print_quad_block(big_block* q);