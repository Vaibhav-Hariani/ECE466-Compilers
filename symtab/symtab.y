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

%nterm <i> enum_constant_def ident_opt
%nterm <c> stgclass_spec qual_spec
%nterm <data> func_spec type_spec
%nterm <data> enum_type_spec enum_type_def enum_type_ref
%nterm <data> struct_type_spec struct_type_def struct_type_ref
%nterm <data> union_type_spec union_type_def union_type_ref
%nterm <data> float_type_spec int_type_spec
%nterm <data> signed_type_spec unsigned_type_spec char_type_spec
%nterm <sym> declaration declaration_spec init_declarator_list
%nterm <sym> component_declaration
%nterm <tab> field_list
%nterm enum_def_list;
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
|	declaration_spec init_declarator_list ';'
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
|	IDENT {$$ = NULL; /*typedef not implemented*/}
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
		ast_sym_t *tag = new_ast_sym($2, STG_NA, SYM_ENUM_T,
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
		$$ = new_ast_sym($1, STG_NA, SYM_ENUM_C, NULL,
			@1.filename, @1.line);
	}
|	enum_def_list ',' enum_constant_def	{
		$$ = new_ast_sym($3, STG_NA, SYM_ENUM_C, NULL,
			@3.filename, @3.line);
		$$->next = $1;
	}
;

enum_constant_def:
	IDENT	{$$ = $1;}
	// IDENT '=' expr // circle back when combining with parser
;

float_type_spec:
	FLOAT	{$$ = new_ast_data(sizeof (float), DATA_SCAL, QUAL_NONE, new_ast_scal(0, SCAL_FLOAT));}
|	DOUBLE	{$$ = new_ast_data(sizeof (double), DATA_SCAL, QUAL_NONE, new_ast_scal(0, SCAL_DOUB));}
|	LONG DOUBLE	{$$ = new_ast_data(sizeof (long double), DATA_SCAL, QUAL_NONE, new_ast_scal(0, SCAL_LONGDOUB));}
// complex not implemented, placeholders below
|	FLOAT COMPLEX	{$$ = new_ast_data(sizeof (float), DATA_SCAL, QUAL_NONE, new_ast_scal(1, SCAL_FLOAT));}
|	DOUBLE COMPLEX	{$$ = new_ast_data(sizeof (double), DATA_SCAL, QUAL_NONE, new_ast_scal(1, SCAL_DOUB));}
|	LONG DOUBLE COMPLEX	{$$ = new_ast_data(sizeof (long double), DATA_SCAL, QUAL_NONE, new_ast_scal(1, SCAL_LONGDOUB));}
;

int_type_spec:
	signed_type_spec	{$$ = $1;}
|	unsigned_type_spec	{$$ = $1;}
|	char_type_spec	{$$ = $1;}
|	BOOL	{$$ = new_ast_data(sizeof (_Bool), DATA_SCAL, QUAL_NONE, new_ast_scal(0, SCAL_FLOAT));}
;

signed_type_spec:
|	signed_opt SHORT int_opt	{$$ = new_ast_data(sizeof (short), DATA_SCAL, QUAL_NONE, new_ast_scal(0, SCAL_SHORT));}
|	signed_opt INT	{$$ = new_ast_data(sizeof (int), DATA_SCAL, QUAL_NONE, new_ast_scal(0, SCAL_INT));}
|	SIGNED	{$$ = new_ast_data(sizeof (int), DATA_SCAL, QUAL_NONE, new_ast_scal(0, SCAL_INT));}
|	signed_opt LONG	int_opt {$$ = new_ast_data(sizeof (long), DATA_SCAL, QUAL_NONE, new_ast_scal(0, SCAL_LONG));}
|	signed_opt LONG LONG int_opt {$$ = new_ast_data(sizeof (long long), DATA_SCAL, QUAL_NONE, new_ast_scal(0, SCAL_LONGLONG));}
;

signed_opt:
	%empty
|	SIGNED
;

unsigned_type_spec:
	UNSIGNED SHORT int_opt	{$$ = new_ast_data(sizeof (unsigned short), DATA_SCAL, QUAL_NONE, new_ast_scal(1, SCAL_SHORT));}
|	UNSIGNED int_opt	{$$ = new_ast_data(sizeof (unsigned int), DATA_SCAL, QUAL_NONE, new_ast_scal(1, SCAL_INT));}
|	UNSIGNED LONG int_opt	{$$ = new_ast_data(sizeof (unsigned long), DATA_SCAL, QUAL_NONE, new_ast_scal(1, SCAL_INT));}
|	UNSIGNED LONG LONG int_opt	{$$ = new_ast_data(sizeof (unsigned long long), DATA_SCAL, QUAL_NONE, new_ast_scal(0, SCAL_LONGLONG));}
;

int_opt:
	%empty
|	INT
;

// note: neutral chars mentioned in h&s, not differentiated below
char_type_spec:
	CHAR	{$$ = new_ast_data(sizeof (char), DATA_SCAL, QUAL_NONE, new_ast_scal(0, SCAL_CHAR));}
|	SIGNED CHAR	{$$ = new_ast_data(sizeof (char), DATA_SCAL, QUAL_NONE, new_ast_scal(0, SCAL_CHAR));}
|	UNSIGNED CHAR	{$$ = new_ast_data(sizeof (unsigned char), DATA_SCAL, QUAL_NONE, new_ast_scal(1, SCAL_CHAR));}
;

struct_type_spec:
	struct_type_def	{$$ = $1;}
|	struct_type_ref	{$$ = $1;}
;

struct_type_def:
	STRUCT '{' field_list '}' {
		$$ = new_ast_data(0, DATA_STRU, QUAL_NONE,
			new_ast_stru(0,
				new_ast_sym(NULL, STG_NA, SYM_STRU_T, NULL,
					@1.filename, @1.line),
				$3));
		
		$$->node->stru->tag->data = $$;
		struct_fix($$);
		$$->node->stru->is_complete = 1;
	}
|	STRUCT IDENT '{' field_list '}' {
		ast_sym_t *temp = lookup(scope_tab, $2, SYM_STRU_T);
		if (temp != NULL && temp->tab == scope_tab) {
			if (temp->data->stru->is_complete) {
				/*ERROR redeclaration not permitted*/
			} else {
				$$ = temp->data;
				$$->data->node->stru->minitab = $4;
			}
		} else {
		$$ = new_ast_data(0, DATA_STRU, QUAL_NONE,
			new_ast_stru(0,
				new_ast_sym($2, STG_NA, SYM_STRU_T, NULL,
					@1.filename, @1.line),
				$4));
			$$->node->stru->tag->data = $$;
			enter(scope_tab, $$->node->stru->tag, 0);
		}
		
		struct_fix($$);
		$$->node->stru->is_complete = 1;
	}
;

struct_type_ref:
	STRUCT IDENT	{
		$$ = lookup(scope_tab, $2, SYM_STRU_T);
		if ($$ == NULL) {
			/*install incomplete tag in symbol table*/
			$$ = new_ast_sym($2, STG_NONE, SYM_STRU_T,
				new_ast_data(0, DATA_STRU, QUAL_NONE,
					new_ast_stru(0, NULL, NULL)),
				@1.filename, @1.line);
			$$->data->node->stru->tag->$$;
		}
	}
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
	component_declaration {
		$$ = new_ast_tab(scope_tab, SCOPE_STRU, @1.filename, @1.line);
		enter($$, $1, 0);
	}
|	field_list component_declaration {
		enter($1, $2, 0);
		$$ = $1;
	}
;

component_declaration:
	type_spec component_declarator_list ';'
;

component_declarator_list:
	component_declarator
|	component_declarator_list ',' component_declarator
;

// bit field width should be constant expression, not NUM
component_declarator:
	declarator
|	':' NUM
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