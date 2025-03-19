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


%nterm <node> primary_expr ast_postfix arg_list
%token <i> IDENT;
%token <c> CHARLIT;
%token <n> NUM;
%token <s> STRING;
%start terminal
%%

terminal: %empty {$$=0;}
| terminal expr ';'  {$$ = print_ast($2);}
;

primary_expr: IDENT {$$=new_ast_ident($1)};
|   CHARLIT  {$$=new_ast_charlit($1)}
|   NUM {$$ = new_ast_num($1);}
|   STRING {$$ = new_ast_string($1);}
|   '(' expr ')' {$$=$1}

ast_postfix: primary_expr {$$=$1;}
|   ast_postfix '[' expr ']' {$$=ast_array_expansion($1, $3);}
|   ast_postfix '(' arg_list ')' {$$=new_ast_binop(AST_function, $1, $3, 0);}
|   ast_postfix '.' IDENT  {$$ = new_ast_binop(AST_binop, $1, new_ast_ident($3), '.');}
|   ast_postfix INDSEL IDENT    { $$=new_ast_binop(AST_binop, $1, new_ast_ident($3), INDSEL);}
|   ast_postfix "++" %prec POSTFIX {$$ = new_ast_unop($1, PLUSPLUS, POSTFIX);}
|   ast_postfix "--" %prec POSTFIX {$$ = new_ast_unop($1, MINUSMINUS, POSTFIX);}
; /* | Typenames go here    */

arg_list: %empty {$$=new_ast_list(0);}
|   assign_expr {$$=new_ast_list($1);}
|   arg_list ',' assign_expr {$$ = append_ast_list{$1,$3};};

ast_unop: ast_postfix {$$=$1;}
|   "++" ast_unop {$$ = new_ast_unop($2, PLUSPLUS, PREFIX);}
|   "--" ast_unop {$$ = new_ast_unop($2, MINUSMINUS, PREFIX);}
|   '+' ast_unop %prec SIZEOF  {$$ = new_ast_unop($2, '+', PREFIX);}
|   '-' ast_unop %prec PREFIX {$$= new_ast_unop($2, '-', PREFIX);}
|   '!' ast_unop %prec SIZEOF  {$$ = new_ast_unop($2, '!', PREFIX);}
|   '~' ast_unop %prec SIZEOF  {$$ = new_ast_unop($2, '~', PREFIX);}
|   '&' ast_unop %prec SIZEOF  {$$ = new_ast_unop($2, '&', PREFIX);}
|    '*' ast_unop %prec SIZEOF {$$ = new_ast_unop($2, '*', PREFIX);}
|   SIZEOF ast_unop {$$ = new_ast_unop($2, SIZEOF, PREFIX);}
;
//Will need later according to c standard
ast_cast:  ast_unop {$$=$1;};

//At least this can be preserved by using the precs from above
ast_binop: ast_cast {$$=$1}
|   ast_binop '+' ast_binop   { $$=new_ast_binop(AST_binop, $1, $3, '+');}
|   ast_binop '-' ast_binop   { $$=new_ast_binop(AST_binop, $1, $3, '-');}
|   ast_binop '*' ast_binop   { $$=new_ast_binop(AST_binop, $1, $3, '*');}
|   ast_binop '/' ast_binop   { $$=new_ast_binop(AST_binop, $1, $3, '/');}
|   ast_binop '%' ast_binop   { $$=new_ast_binop(AST_binop, $1, $3, '%');}
|   ast_binop '>' ast_binop   { $$=new_ast_binop(AST_binop, $1, $3, '>');}
|   ast_binop '<' ast_binop   { $$=new_ast_binop(AST_binop, $1, $3, '<');}
|	ast_binop '&' ast_binop 	 { $$=new_ast_binop(AST_binop, $1, $3, '&');}
|	ast_binop '|' ast_binop 	 { $$=new_ast_binop(AST_binop, $1, $3, '|');}
|   ast_binop '^' ast_binop   { $$=new_ast_binop(AST_binop, $1, $3, '^');}
|   ast_binop SHL ast_binop 	    { $$=new_ast_binop(AST_binop, $1, $3, SHL);}
|	ast_binop SHR ast_binop 	    { $$=new_ast_binop(AST_binop, $1, $3, SHR);}
|	ast_binop EQEQ ast_binop 	     { $$=new_ast_binop(AST_binop, $1, $3, EQEQ);}
|	ast_binop NOTEQ ast_binop 	 { $$=new_ast_binop(AST_binop, $1, $3, NOTEQ);}
|	ast_binop LOGAND ast_binop 	 { $$=new_ast_binop(AST_binop, $1, $3, LOGAND);}
|	ast_binop LOGOR ast_binop 	 { $$=new_ast_binop(AST_binop, $1, $3, LOGOR);}
|   ast_binop LTEQ ast_binop 	 { $$=new_ast_binop(AST_binop, $1, $3, LTEQ);}
|	ast_binop GTEQ ast_binop 	 { $$=new_ast_binop(AST_binop, $1, $3, GTEQ);}

ast_assign: ast_binop {$$=$1};
|   expr '=' expr   { $$=new_ast_binop(AST_assign, $1, $3, '=');}
|	expr TIMESEQ expr 	 { $$=new_ast_binop(AST_assign, $1, $3, TIMESEQ);}
|	expr DIVEQ expr 	 { $$=new_ast_binop(AST_assign, $1, $3, DIVEQ);}
|	expr MODEQ expr 	 { $$=new_ast_binop(AST_assign, $1, $3, MODEQ);}
|	expr PLUSEQ expr 	 { $$=new_ast_binop(AST_assign, $1, $3, PLUSEQ);}
|	expr MINUSEQ expr 	 { $$=new_ast_binop(AST_assign, $1, $3, MINUSEQ);}
|	expr SHLEQ expr 	 { $$=new_ast_binop(AST_assign, $1, $3, SHLEQ);}
|	expr SHREQ expr 	 { $$=new_ast_binop(AST_assign, $1, $3, SHLEQ);}
|	expr ANDEQ expr 	 { $$=new_ast_binop(AST_assign, $1, $3, ANDEQ);}
|	expr OREQ expr 	     { $$=new_ast_binop(AST_assign, $1, $3, OREQ);}
|	expr XOREQ expr 	 { $$=new_ast_binop(AST_assign, $1, $3, XOREQ);}
;
expr: ast_assign
|   ast_assign ',' ast_assign




ast_binop: expr '+' expr   { $$=new_ast_binop(AST_binop, $1, $3, '+');}
|   expr '-' expr   { $$=new_ast_binop(AST_binop, $1, $3, '-');}
|   expr '*' expr   { $$=new_ast_binop(AST_binop, $1, $3, '*');}
|   expr '/' expr   { $$=new_ast_binop(AST_binop, $1, $3, '/');}
|   expr '%' expr   { $$=new_ast_binop(AST_binop, $1, $3, '%');}
|   expr '>' expr   { $$=new_ast_binop(AST_binop, $1, $3, '>');}
|   expr '<' expr   { $$=new_ast_binop(AST_binop, $1, $3, '<');}
|	expr '&' expr 	 { $$=new_ast_binop(AST_binop, $1, $3, '&');}
|	expr '|' expr 	 { $$=new_ast_binop(AST_binop, $1, $3, '|');}
|   expr '^' expr   { $$=new_ast_binop(AST_binop, $1, $3, '^');}
|   expr INDSEL IDENT    { $$=new_ast_binop(AST_binop, $1, new_ast_ident($3), INDSEL);}
|   expr '.' IDENT  {$$ = new_ast_binop(AST_binop, $1, new_ast_ident($3), '.');}
|   expr SHL expr 	 { $$=new_ast_binop(AST_binop, $1, $3, SHL);}
|	expr SHR expr 	 { $$=new_ast_binop(AST_binop, $1, $3, SHR);}
|	expr EQEQ expr 	     { $$=new_ast_binop(AST_binop, $1, $3, EQEQ);}
|	expr NOTEQ expr 	 { $$=new_ast_binop(AST_binop, $1, $3, NOTEQ);}
|	expr LOGAND expr 	 { $$=new_ast_binop(AST_binop, $1, $3, LOGAND);}
|	expr LOGOR expr 	 { $$=new_ast_binop(AST_binop, $1, $3, LOGOR);}
|   expr LTEQ expr 	 { $$=new_ast_binop(AST_binop, $1, $3, LTEQ);}
|	expr GTEQ expr 	 { $$=new_ast_binop(AST_binop, $1, $3, GTEQ);}
|   expr ',' expr    { $$=new_ast_binop(AST_binop, $1, $3, ',');}
//Helper function that should expand this into what it actually is
/* |   expr '[' ']' {$$=ast_array_expansion($1, 0)}; */
|   expr '[' expr ']'  {$$ =ast_array_expansion($1, $3);}
//Special function object type: Treated like a binop in the tree but expands out later
|   expr '('ast_list')' { $$=new_ast_binop(AST_function, $1, $3, 0);};
;

//separated here so theat lvalues can be handled properly later on in the system
ast_assign: 


ast_ternop: expr '?' expr ':' expr {$$=new_ast_ternop(AST_ternop, $1, $3, $5);};



ast_unop: expr "++" %prec POSTFIX {$$ = new_ast_unop($1, PLUSPLUS, POSTFIX);}
|   expr "--" %prec POSTFIX {$$ = new_ast_unop($1, MINUSMINUS, POSTFIX);}
|   "++" expr %prec PREFIX {$$ = new_ast_unop($2, PLUSPLUS, PREFIX);}
|   "--" expr %prec PREFIX {$$ = new_ast_unop($2, MINUSMINUS, PREFIX);}
|   '+' expr %prec SIZEOF  {$$ = new_ast_unop($2, '+', PREFIX);}
|   '-' expr %prec PREFIX {$$= new_ast_unop($2, '-', PREFIX);}
|   '!' expr %prec SIZEOF  {$$ = new_ast_unop($2, '!', PREFIX);}
|   '~' expr %prec SIZEOF  {$$ = new_ast_unop($2, '~', PREFIX);}
|   '&' expr %prec SIZEOF  {$$ = new_ast_unop($2, '&', PREFIX);}
|    '*' expr %prec SIZEOF {$$ = new_ast_unop($2, '*', PREFIX);}
|   SIZEOF expr {$$ = new_ast_unop($2, SIZEOF, PREFIX);}
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