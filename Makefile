
CFLAGS = -Wall -Wno-abi -I ./lexer -I ./parser -I ./symtab

all: parser

flex: lexer/lex.l
	cd lexer && flex lex.l

bison_parser: parser/ast_nodes.c parser/ast_nodes.h parser/parser.y parser/parse_output.h
	cd parser && bison -d parser.y

bison_parser_debug: parser/ast_nodes.c parser/ast_nodes.h parser/parser.y
	cd parser && bison --debug -d parser.y 

parser: flex bison
	gcc -o parser/parser.out $(CFLAGS) parser/parser.tab.c lexer/lex.yy.c parser/ast_nodes.c parser/parse_output.c

debug_parser: flex bison_debug
	gcc -o parser/parser_debug.out $(CFLAGS) -g parser/parser.tab.c lexer/lex.yy.c parser/ast_nodes.c parser/parse_output.c

bison_symtab: symtab/symbol.c symtab/symbol.h symtab/table.c symtab/table.h symtab/data.c symtab/data.h symtab/symtab.y symtab/symtab_output.h parser/ast_nodes.c parser/ast_nodes.h
	cd symtab && bison -Wcounterexamples -Wno-yacc -d symtab.y

bison_symtab_debug: symtab/symbol.c symtab/symbol.h symtab/table.c symtab/table.h symtab/data.c symtab/data.h symtab/symtab.y parser/ast_nodes.c parser/ast_nodes.h
	cd symtab && bison -Wno-yacc --debug -d symtab.y 

symtab: flex bison_symtab
	gcc -o symtab/symtab.out $(CFLAGS) symtab/symtab.tab.c lexer/lex.yy.c symtab/symbol.c symtab/table.c symtab/data.c symtab/symtab_output.c

debug_symtab: flex bison_symtab_debug
	gcc -o symtab/symtab_debug.out $(CFLAGS) -g symtab/symtab.tab.c lexer/lex.yy.c symtab/symbol.c symtab/table.c symtab/data.c symtab/symtab_output.c

clean:
	rm -rf *.h.gch parser/*.out symtab/*.out lexer/lex.yy.c parser/parser.tab.* symtab/symtab.tab.*
