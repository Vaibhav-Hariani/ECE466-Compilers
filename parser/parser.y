%{
     #include "parser.tab.h"
     int yylex(void);
     extern FILE *yyin;
 %}


//Undefined token, but lets parser.tab.h include everything
%token
TOKEOF	ELLIPSIS	
POUNDPOUND	AUTO	BREAK	CASE	CHAR	CONST	
CONTINUE	DEFAULT	DO	DOUBLE	ELSE	ENUM	
EXTERN	FLOAT	FOR	GOTO	IF	INLINE	INT	LONG	
REGISTER	RESTRICT	RETURN	SHORT	SIGNED	STATIC	
STRUCT	SWITCH	TYPEDEF	UNION	UNSIGNED	VOID	
VOLATILE	WHILE	BOOL	COMPLEX	IMAGINARY
%token 
    PLUSPLUS "++" 
    MINUSMINUS "--"
%left ','
%right '=' PLUSEQ MINUSEQ DIVEQ TIMESEQ MODEQ SHLEQ SHREQ ANDEQ OREQ XOREQ
%right '?' ':'	/* This is where yacc will put it */
%left LOGOR
%left LOGAND
%left '|'
%left '^'
%left '&' 
%left  EQEQ NOTEQ
%left GTEQ LTEQ '>' '<'
%left SHL SHR
%left '+' '-'
%left '*' '/' '%'
%right SIZEOF PREFIX '!' '~' 
%left '(' ')' '[' ']'  POSTFIX PLUSPLUS MINUSMINUS INDSEL '.'

%union {
	char *i;
	char c;
	TypedNumber n;
    SizedString s;
    ast_node *node;
} 
%code requires {
    #include "yylval.h"
    #include "ast_nodes.h"
    #include <stdio.h>
    #include "parse_output.h"
    void yyerror(const char * s);
}


%nterm <node> prog term_expr expr binop_expr ternop_expr unop_expr assign_expr arg_list 
%token <i> IDENT;
%token <c> CHARLIT;
%token <n> NUM;
%token <s> STRING;
%start prog
%%
// 2 types of elements
// expr, and lvalues
// lvalues can be a type of expr, that reduce to an ident, or an address in memory
// as such, they can be ternarys, pointers, or identifiers. Nothing else.
//unops take in an lvalue, and do one of 3 operations to them.

prog: %empty {$$=0;}
| prog term_expr ';'  {$$ = print_ast($2);}
;

term_expr: assign_expr {$$=$1;}
|   term_expr ',' assign_expr {$$ = new_ast_double(AST_binop, $1, $3, ',');} 

expr: NUM {$$ = new_ast_num($1);}
|   IDENT {$$ = new_ast_ident($1);}
|   CHARLIT {$$ = new_ast_charlit($1);}
|   STRING {$$ = new_ast_string($1);}
|   '(' term_expr ')'            {$$=$2;}
;

unop_expr: expr {$$=$1;}
|   unop_expr "++" %prec POSTFIX { $$ = new_ast_single($1, PLUSPLUS, POSTFIX);}
|   unop_expr "--" %prec POSTFIX { $$ = new_ast_single($1, MINUSMINUS, POSTFIX);}
|   "++" unop_expr %prec PREFIX  { $$ = new_ast_single($2, PLUSPLUS, PREFIX);}
|   "--" unop_expr %prec PREFIX  { $$ = new_ast_single($2, MINUSMINUS, PREFIX);}
|   unop_expr INDSEL IDENT       { $$=new_ast_double(AST_binop, $1, new_ast_ident($3), INDSEL);}
|   unop_expr '.' IDENT          { $$ = new_ast_double(AST_binop, $1, new_ast_ident($3), '.');}
//Helper function that should expand this into what it actually is
|   unop_expr '['term_expr']'    { $$= ast_array_exp($1,$3);}
|   unop_expr '(' arg_list ')'   { $$=new_ast_double(AST_funct, $1, $3, ')');}
|   '+' unop_expr %prec SIZEOF   { $$ = new_ast_single($2, '+', PREFIX);}
|   '-' unop_expr %prec SIZEOF   { $$= new_ast_single($2, '-', PREFIX);}
|   '!' unop_expr %prec SIZEOF   { $$ = new_ast_single($2, '!', PREFIX);}
|   '~' unop_expr %prec SIZEOF   { $$ = new_ast_single($2, '~', PREFIX);}
|   '&' unop_expr %prec SIZEOF   { $$ = new_ast_single($2, '&', PREFIX);}
|    '*' unop_expr %prec SIZEOF  { $$ = new_ast_single($2, '*', PREFIX);}
|   SIZEOF unop_expr {$$ = new_ast_single($2, SIZEOF, PREFIX);}
;


binop_expr: unop_expr {$$=$1;}
|   binop_expr '+' binop_expr        { $$=new_ast_double(AST_binop, $1, $3, '+');}
|   binop_expr '-' binop_expr        { $$=new_ast_double(AST_binop, $1, $3, '-');}
|   binop_expr '*' binop_expr        { $$=new_ast_double(AST_binop, $1, $3, '*');}
|   binop_expr '/' binop_expr        { $$=new_ast_double(AST_binop, $1, $3, '/');}
|   binop_expr '%' binop_expr        { $$=new_ast_double(AST_binop, $1, $3, '%');}
|   binop_expr '>' binop_expr        { $$=new_ast_double(AST_binop, $1, $3, '>');}
|   binop_expr '<' binop_expr        { $$=new_ast_double(AST_binop, $1, $3, '<');}
|	binop_expr '&' binop_expr 	     { $$=new_ast_double(AST_binop, $1, $3, '&');}
|	binop_expr '|' binop_expr 	     { $$=new_ast_double(AST_binop, $1, $3, '|');}
|   binop_expr '^' binop_expr        { $$=new_ast_double(AST_binop, $1, $3, '^');}
|   binop_expr SHL binop_expr 	     { $$=new_ast_double(AST_binop, $1, $3, SHL);}
|	binop_expr SHR binop_expr  	     { $$=new_ast_double(AST_binop, $1, $3, SHR);}
|	binop_expr EQEQ binop_expr  	 { $$=new_ast_double(AST_binop, $1, $3, EQEQ);}
|	binop_expr NOTEQ binop_expr  	 { $$=new_ast_double(AST_binop, $1, $3, NOTEQ);}
|	binop_expr LOGAND binop_expr  	 { $$=new_ast_double(AST_binop, $1, $3, LOGAND);}
|	binop_expr LOGOR binop_expr  	 { $$=new_ast_double(AST_binop, $1, $3, LOGOR);}
|   binop_expr LTEQ binop_expr  	 { $$=new_ast_double(AST_binop, $1, $3, LTEQ);}
|	binop_expr GTEQ binop_expr  	 { $$=new_ast_double(AST_binop, $1, $3, GTEQ);}

ternop_expr: binop_expr { $$=$1;}
| binop_expr '?' binop_expr ':' binop_expr { $$=new_ast_ternop(AST_ternop, $1, $3, $5);};

assign_expr: ternop_expr {$$=$1;}
|   unop_expr '=' assign_expr      { $$=new_ast_double(AST_assign, $1, $3, '=');}
|	unop_expr TIMESEQ assign_expr	 { $$=new_ast_double(AST_assign, $1, $3, TIMESEQ);}
|	unop_expr DIVEQ assign_expr 	 { $$=new_ast_double(AST_assign, $1, $3, DIVEQ);}
|	unop_expr MODEQ assign_expr 	 { $$=new_ast_double(AST_assign, $1, $3, MODEQ);}
|	unop_expr PLUSEQ assign_expr 	 { $$=new_ast_double(AST_assign, $1, $3, PLUSEQ);}
|	unop_expr MINUSEQ assign_expr  { $$=new_ast_double(AST_assign, $1, $3, MINUSEQ);}
|	unop_expr SHLEQ assign_expr 	 { $$=new_ast_double(AST_assign, $1, $3, SHLEQ);}
|	unop_expr SHREQ assign_expr 	 { $$=new_ast_double(AST_assign, $1, $3, SHLEQ);}
|	unop_expr ANDEQ assign_expr 	 { $$=new_ast_double(AST_assign, $1, $3, ANDEQ);}
|	unop_expr OREQ assign_expr 	 { $$=new_ast_double(AST_assign, $1, $3, OREQ);}
|	unop_expr XOREQ assign_expr 	 { $$=new_ast_double(AST_assign, $1, $3, XOREQ);}
;

arg_list: %empty { $$ = new_ast_list(0);} 
|   assign_expr { $$=new_ast_list($1);}
|   arg_list ',' assign_expr %prec POSTFIX { $$ = append_ast_list($1, $3);}
;




/* keyword: STRUCT IDENT
| CHAR
| BOOL    :) slomp blup 
| COMPLEX
| mult_keyword COMPLEX
| IMAGINARY
| mult_keyword IMAGINARY

| 

mult_keyword: LONG
| DOUBLE
| FLOAT
| mult_keyword mult_keyword

;

storage_class: EXTERN
|   STRUCT
|   STATIC
|   VOLATILE
|   SIGNED
|   UNSIGNED
|   storage_class storage_class
|    */
%%

/* #ifdef YYDEBUG
    extern int yydebug = 1;
#endif */


void yyerror(const char *s){
    fprintf(stderr, "Error: %s \n", s);
}

int main(int argc, char** argv){
    FILE *file;
    if(argc < 2) {
        /* yyerror("No File Specified");
        return 0; */
        yyin = stdin;
        fprintf(stderr, "No File Specified \n");
    } else {
        file = fopen(argv[1],"r");
        if(!file) {
            yyerror("No valid file specified");
            return 0;
        }
        yyin = file;
    }
    yyparse();
        if(yyin != stdin) {
        fclose(file);    
    }

    return 0;
    /* yyparse();  */
} 