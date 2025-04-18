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
    ast_data_t *data;
    ast_sym_t *sym;
    ast_tab_t *tab;
	struct ast_scal *scal;
	struct ast_var *var;
	struct ast_ptr *ptr;
	struct ast_ary *ary;
	struct ast_param *param;
	struct ast_func *func;
	struct ast_stru *stru;
	struct ast_unio *unio;
	struct ast_enu *enu;
	struct ast_label *label;
}

%code requires {
    #include "yylval.h"
    #include "ast_nodes.h"
    #include "ast_symtab.h"
    #include <stdio.h>
    #include "parse_output.h"
    #include "symtab_output.h"
    void yyerror(const char * s);

	#define YYLTYPE YYLTYPE
	typedef struct YYLTYPE {
		int line;
		char *filename;
	} YYLTYPE;

	sym_tab_t *scope_tab;
}

%define api.location.type {YYLTYPE}
%locations

%nterm <i> enum_constant_def ident_opt;
%nterm <c> stgclass_spec qual_spec;
%nterm <node> prog term_expr expr binop_expr ternop_expr unop_expr assign_expr;
%nterm <data> type_spec func_spec enum_type_spec enum_type_def enum_type_ref float_type_spec int_type_spec struct_type_spec union_type_spec;
%nterm <sym> declaration_spec init_declarator_list enum_def_list;
%nterm <tab>;
%nterm <scal>;
%nterm <var>;
%nterm <ptr>;
%nterm <ary>;
%nterm <param>;
%nterm <func>;
%nterm <stru>;
%nterm <unio>;
%nterm <enu>;
%nterm <label>;
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

// prog: %empty {$$=0;}
// | prog term_expr ';'  {$$ = print_ast($2);}
// ;

declaration:
	declaration_spec ';'
|	declaration_spec init_declatator_list ';'
;

// declaration specifiers

declaration_spec:
	stgclass_spec	{
		$$ = new_ast_sym(NULL, $1, SYM_NONE, NULL,
			@1.filename, @1.line);
		}
|	type_spec	{
		$$ = new_ast_sym(NULL, STG_NONE, SYM_NONE, $1,
			@1.filename, @1.line);
		}
|	qual_spec	{
		$$ = new_ast_sym(NULL, STG_NONE, SYM_NONE,
			new_ast_data(0, DATA_NONE, $1, NULL),
			@1.filename, @1.line);
		}
|	func_spec	{
		$$ = new_ast_sym(NULL, STG_NONE, SYM_FUNC,
			new_ast_data(0, DATA_FUNC, QUAL_NONE, NULL),
			@1.filename, @1.line);
		$$.is_inline = 1;
		}
|	stgclass_spec declaration_spec {
		if ($2->stg_type != STG_NONE && $2->stg_type != $1) {
			/*ERROR*/
		} else {
			$2->stg_type = $1;
		}
		$$ = $2;
	}
|	qual_spec declaration_spec {
		if ($2->data == NULL) {
			$2->data = new_ast_data(0, DATA_NONE, $1, NULL);
		} else {
			$2->data->qual |= $1;
		}
		$$ = $2;
	}
|	func_spec declaration_spec {
		if ($2->sym_type != SYM_NONE && $2->sym_type != SYM_FUNC) {
			/*ERROR*/
		} else {
			$2->sym_type = SYM_FUNC;
		}
		/* if ($2->data == NULL) {
			$2->data = new_ast_data(0, DATA_FUNC, QUAL_NONE, NULL);
		} else {
			$2->data->data_type = DATA_FUNC;
		}*/
		$2->is_inline = 1;
		$$ = $2;
	}
|	type_spec declaration_spec {
		if ($2->data != NULL) {
			/*ERROR*/
		} else {
			$2->data = $1;
		}
		$$ = $2;
	}	
;

stgclass_spec:
	EXTERN	{$$ = (char) STG_EXTERN_EXP;}
|	STATIC	{$$ = (char) STG_EXTERN_EXP;}
|	AUTO	{$$ = (char) STG_EXTERN_EXP;}
|	REGISTER	{$$ = (char) STG_EXTERN_EXP;}
|	TYPEDEF	{$$ = (char) STG_TYPEDEF;}
;

qual_spec:
|	CONST	{$$ = (char) QUAL_CONST;}
|	VOLATILE	{$$ = (char) QUAL_VOLATILE;}
|	RESTRICT	{$$ = (char) QUAL_RESTRICT;}
;

func_spec:
	INLINE	{$$ = /*func type with is_inline = 1*/}
;

type_spec:
	enum_type_spec	{$$ = $1;}
|	float_type_spec	{$$ = $1;}
|	int_type_spec	{$$ = $1;}
|	struct_type_spec	{$$ = $1;}
|	union_type_spec	{$$ = $1;}
|	VOID	{$$ = new_ast_data(0, DATA_SCAL, QUAL_NONE, new_ast_scal(0, SCAL_VOID));}
|	IDENT /*typedef not implemented*/
;

enum_type_spec:
	enum_type_def	{$$ = $1;}
|	enum_type_ref	{$$ = $1;}
;

enum_type_def:
	ENUM '{' enum_def_list comma_opt '}' {
		$$ = new_ast_data(sizeof (int), DATA_ENU, QUAL_NONE,
			new_ast_enu(NULL));
		$$->next = $3;
	}
|	ENUM IDENT '{' enum_def_list comma_opt '}' {
		ast_sym_t *tag = new_ast_sym($2, STG_NONE, SYM_ENUM_T,
				new_ast_data(sizeof (int), DATA_ENU, QUAL_NONE, NULL));
		tag->data->node->enu->tag = tag; /*this is ridiculous*/
		$$ = tag->data;
		$$->next = $4;
	}
;

enum_type_ref:
	ENUM IDENT	{
		$$ = lookup(scope_tab, $2, SYM_ENUM_T);
		if ($$ == NULL) {
			/*no forward references allowed for enums, so:*/
			/*ERROR*/
		}
	}
;

enum_def_list:
	enum_constant_def	{
		$$ = new_ast_sym($1, STGCLASS_NA, SYM_ENUM_C, NULL,
			@1.filename, @1.line);
	}
|	enum_def_list ',' enum_constant_def	{
		$$ = new_ast_sym($3, STGCLASS_NA, SYM_ENUM_C, NULL,
			@3.filename, @3.line);
		$$->next = $1;
	}
;

enum_constant_def:
	IDENT	{$$ = $1;}
	// IDENT '=' expr // circle back when combining with parser
;

float_type_spec:
	FLOAT
|	DOUBLE
|	LONG DOUBLE
|	FLOAT COMPLEX
|	DOUBLE COMPLEX
|	LONG DOUBLE COMPLEX
;

int_type_spec:
	signed_type_spec
|	unsigned_type_spec
|	char_type_spec
|	BOOL
;

signed_type_spec:
	SHORT
|	SHORT INT
|	SIGNED SHORT
|	SIGNED SHORT INT
|	INT
|	SIGNED INT
|	SIGNED
|	LONG
|	LONG INT
|	SIGNED LONG
|	SIGNED LONG INT
|	LONG LONG
|	LONG LONG INT
|	SIGNED LONG LONG
|	SIGNED LONG LONG INT
;

unsigned_type_spec:
	UNSIGNED SHORT
|	UNSIGNED SHORT INT
|	UNSIGNED INT
|	UNSIGNED
|	UNSIGNED LONG
|	UNSIGNED LONG INT
|	UNSIGNED LONG LONG
|	UNSIGNED LONG LONG INT
;

char_type_spec:
	CHAR
|	SIGNED CHAR
|	UNSIGNED CHAR
;

struct_type_spec:
	struct_type_def
|	struct_type_ref
;

struct_type_def:
	STRUCT '{' field_list '}'
|	STRUCT IDENT '{' field_list '}'
;

struct_type_ref:
	STRUCT IDENT
;

union_type_spec:
	union_type_def
|	union_type_ref
;

union_type_def:
	UNION '{' field_list '}'
|	UNION IDENT '{' field_list '}'
;

union_type_ref:
	UNION IDENT
;
field_list:
	component_declaration
|	field_list component_declaration
;

component_declaration:
	type_spec component_declarator_list ';'
;

component_declarator_list:
	component_declarator
|	component_declarator_list ',' component_declarator
;

component_declarator:
	declarator
|	':' NUM // bit field width should be constant expression, not NUM
|	declarator ':' NUM
;

type_name:
	declaration_spec
|	declaration_spec abstract_declarator
;

// declarators

init_declarator_list:
	init_declarator_list
|	init_declarator_list ',' init_declarator
;

init_declarator:
	declarator
// |	declarator '=' init // circle back later, h&s 4.6. involves parser
;

declarator:
	pointer_declarator
|	direct_declarator
;

pointer_declarator:
	pointer direct_declarator
;

pointer:
	'*'
|	'*' qual_spec_list
|	'*' pointer
|	'*' qual_spec_list pointer
;

qual_spec_list:
	qual_spec
|	qual_spec_list qual_spec
;

direct_declarator:
	simple_declarator
|	'(' declarator ')'
|	array_declarator
|	function_declarator
;

simple_declarator:
	IDENT
;

array_declarator:
	direct_declarator '[' ']'
|	direct_declarator '[' NUM ']' // our simplification
;

function_declarator:
	direct_declarator '{' '}'
|	direct_declarator '{' param_type_list '}'
|	direct_declarator '{' ident_list '}'	
;

param_type_list:
	param_list
|	param_list ',' '...'
;

param_list:
	param_declaration
|	param_list ',' param_declaration
;

param_declaration:
	declaration_spec declarator
|	declaration_spec abstract_declarator
|	declaration_spec
;

abstract_declarator:
	pointer
|	pointer direct_abstract_declarator
|	direct_abstract_declarator
;

direct_abstract_declarator:
	'(' abstract_declarator ')'
|	'[' ']'
|	'[' NUM ']'
|	direct_abstract_declarator '[' ']'
|	direct_abstract_declarator '[' NUM ']'
|	'(' ')'
|	'(' param_type_list')'
|	direct_abstract_declarator '(' ')'
|	direct_abstract_declarator '(' param_type_list')'
;	

ident_list:
	IDENT
|	param_list ',' IDENT
;

ident_opt:
	%empty	{$$ = NULL;}
|	IDENT	{$$ = $1;}
;

comma_opt:
	%empty
|	','
;

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