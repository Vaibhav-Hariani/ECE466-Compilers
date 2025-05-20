# Compiler flags: -Wall (all warnings), -Wno-abi (suppress ABI warnings),
# -I flags to specify include directories for header files.
CFLAGS = -Wall -Wno-abi -I ./lexer -I ./parser -I ./parser/util -I ./symtab/out -I ./parser/out -I ./quads/util -I ./quads/out

# Variables for C source files
PARSER_UTIL_C = parser/util/expr.c parser/util/stmt.c parser/util/table.c parser/util/symbol.c parser/util/data.c
PARSER_OUT_C = parser/out/expr_out.c parser/out/stmt_out.c parser/out/symtab_out.c
QUADS_UTIL_C = quads/util/quads.c
QUADS_OUT_C =  quads/out/quads_out.c

# Variables for H header files (primarily for dependency tracking in rules like 'bison')
PARSER_UTIL_H = parser/util/expr.h parser/util/stmt.h parser/util/table.h parser/util/symbol.h parser/util/data.h
PARSER_OUT_H = parser/out/expr_out.h parser/out/stmt_out.h parser/out/symtab_out.h # Corrected stmt_out.c to stmt_out.h
QUADS_UTIL_H = quads/util/quads.h
QUADS_OUT_H = quads/out/quads_out.h

# Default target
all: parser test_quads

# Rule to generate the lexer using Flex
flex: lexer/lex.l
	cd lexer && flex lex.l

# Rule to generate the parser using Bison
# Depends on parser.y and relevant header files to ensure regeneration if they change.
bison: parser/parser.y $(PARSER_UTIL_H) $(PARSER_OUT_H) $(QUADS_UTIL_H)
	cd parser && bison -d parser.y

# Rule to generate the parser with debug information using Bison
bison_debug: parser/parser.y $(PARSER_UTIL_H) $(PARSER_OUT_H) $(QUADS_UTIL_H)
	cd parser && bison --debug -d parser.y 

# Rule to build the main parser executable
# Depends on flex, bison, and relevant C source files.
# Removed QUADS dependencies as they are not in the gcc link line for parser.out
parser: flex bison $(PARSER_UTIL_C) $(PARSER_OUT_C) $(QUADS_UTIL_C)
	gcc -o parser/parser.out $(CFLAGS) parser/parser.tab.c lexer/lex.yy.c $(PARSER_UTIL_C) $(PARSER_OUT_C)

# Rule to build the debug version of the parser executable
# Assumes parser_debug.out needs QUADS_UTIL_C but not QUADS_OUT_C
debug_parser: flex bison_debug $(PARSER_UTIL_C) $(PARSER_OUT_C) $(QUADS_UTIL_C) $(QUADS_OUT_C)
	gcc -o parser/parser_debug.out $(CFLAGS) -g parser/parser.tab.c lexer/lex.yy.c $(PARSER_UTIL_C) $(PARSER_OUT_C) $(QUADS_UTIL_C)  $(QUADS_OUT_C)

# Rule to build the quads test executable
# Corrected: Removed explicit parser/util/expr.c from gcc command.
# Corrected: Removed header file variables from gcc command.
# Added parser/parser.tab.c and lexer/lex.yy.c to gcc command and dependencies.
test_quads: flex bison quads/quads_test.c parser/parser.tab.c lexer/lex.yy.c $(PARSER_UTIL_C) $(PARSER_OUT_C) $(QUADS_UTIL_C) $(QUADS_OUT_C)
	gcc -o quads/test.out $(CFLAGS) -g quads/quads_test.c parser/parser.tab.c lexer/lex.yy.c $(PARSER_UTIL_C) $(PARSER_OUT_C) $(QUADS_UTIL_C) $(QUADS_OUT_C)

# Rule to clean up generated files
clean:
	rm -rf *.h.gch parser/*.out lexer/lex.yy.c parser/parser.tab.* quads/*.out *.out parser_debug.out

