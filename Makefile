
CFLAGS = -Wall -Wno-abi -I ./lexer -I ./parser -I ./parser/util -I ./symtab/out
PARSER_UTIL_C = parser/util/expr.c parser/util/stmt.c parser/util/table.c parser/util/symbol.c parser/util/data.c
PARSER_UTIL_H = parser/util/expr.h parser/util/stmt.h parser/util/table.h parser/util/symbol.h parser/util/data.h
PARSER_OUT_C = parser/out/expr_out.c parser/out/symtab_out.c
PARSER_OUT_H = parser/out/expr_out.h parser/out/symtab_out.h

all: parser

flex: lexer/lex.l
	cd lexer && flex lex.l

bison: parser/parser.y $(PARSER_UTIL_H) $(PARSER_OUT_H)
	cd parser && bison -d parser.y

bison_debug: parser/parser.y $(PARSER_UTIL_H) $(PARSER_OUT_H)
	cd parser && bison --debug -d parser.y 

parser: flex bison $(PARSER_UTIL_C) $(PARSER_OUT_C)
	gcc -o parser/parser.out $(CFLAGS) parser/parser.tab.c lexer/lex.yy.c $(PARSER_UTIL_C) $(PARSER_OUT_C)

debug_parser: flex bison_debug $(PARSER_UTIL_C) $(PARSER_OUT_C)
	gcc -o parser/parser_debug.out $(CFLAGS) -g parser/parser.tab.c lexer/lex.yy.c $(PARSER_UTIL_C) $(PARSER_OUT_C)

clean:
	rm -rf *.h.gch parser/*.out lexer/lex.yy.c parser/parser.tab.*
