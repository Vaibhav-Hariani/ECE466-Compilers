%{
     #include "symtab.tab.h"
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
    #include "table.h"
    #include "symbol.h"
    #include "data.h"
    #include <stdio.h>
	#include <string.h>
    #include "symtab_output.h"
    void yyerror(const char * s);

	#define YYLTYPE YYLTYPE
	typedef struct YYLTYPE {
		int first_line;
		int first_column;
		int last_line;
		int last_column;
		char *filename;
	} YYLTYPE;
}

%code {
	ast_tab_t *tab = NULL;
}

%define api.location.type {YYLTYPE}
%locations

%nterm <i> enum_constant_def;
%nterm <c> stgclass_spec qual_spec qual_spec_list;
%nterm <n> num_opt;
%nterm <data> float_type_spec int_type_spec;
%nterm <data> signed_type_spec unsigned_type_spec char_type_spec;
%nterm <data> pointer;
%nterm <data> type_name;
%nterm <sym> prog declaration_or_fndef function_def;
%nterm <sym> compound_statement decl_or_stmt decl_or_stmt_list statement
%nterm <sym> declaration declaration_spec untyped_declaration_spec param_declaration;
%nterm <sym> init_declarator init_declarator_list;
%nterm <sym> type_spec;
%nterm <sym> enum_type_spec enum_type_def enum_type_ref enum_def_list;
%nterm <sym> struct_type_spec struct_type_def struct_type_ref;
%nterm <sym> union_type_spec union_type_def union_type_ref
%nterm <sym> component_declaration component_declarator component_declarator_list;
%nterm <sym> declarator declarator_list;
%nterm <sym> direct_declarator array_declarator function_declarator;
%nterm <sym> param_declarator abstract_declarator direct_abstract_declarator;
%nterm <sym> field_list param_list;
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

prog:
	%empty	{tab = new_table(0); $$ = NULL;}
|	declaration_or_fndef	{$$ = NULL;}
;

declaration_or_fndef:
	declaration {
		switch ($1->stg_type) {
			case STG_EXTERN_EXP:
			case STG_STATIC:
				$1->stg_type = $1->stg_type;
				break;
			case STG_NONE:
				$1->stg_type = STG_EXTERN_IMP;
				break;
			default:
				/*ERROR invalid storage class specifier*/
				$1->stg_type = STG_EXTERN_IMP;
				break;
		}

		if (!insert_list(tab, $1, SCO_FILE, __INT_MAX__, 1)) {
			$$ = $1;
		} else {
			$$ = NULL;
		}
	}
|	function_def {$$ = $1;}
;

function_def: /*does not support k&r style*/
	declaration_spec function_declarator compound_statement {
		if ($1->data->qual != QUAL_NONE) {
			/*ERROR qualifiers invalid*/
		}

		else if ($2->data->data_type != DATA_FUNC) {
			/*ERROR expected a semicolon after declarator?*/
		}

		else {
			switch ($1->stg_type) {
				case STG_EXTERN_EXP:
				case STG_STATIC:
					$2->stg_type = $1->stg_type;
					break;
				case STG_NONE:
					$2->stg_type = STG_EXTERN_IMP;
					break;
				default:
					/*ERROR invalid storage class specifier*/
					$2->stg_type = STG_EXTERN_IMP;
					break;
			}

			$2->sym_type = SYM_FUNC;
			$2->data->node->func->is_complete = 1;
			$2->tail = install_tail($2, $1->data);
			del_ast_sym($1);

			if (!insert(tab, $2, SCO_FILE, __INT_MAX__, 1)) {	// insert function
				insert_list(tab, $2->data->node->func->params, SCO_FUNC,
					@3.last_line, 1);	// insert params
				insert_list(tab, $3, SCO_FUNC,
					@3.last_line, 1);	// insert compound statement
				$$ = $2;
			} else {
				del_sym_list($3);
				$$ = NULL;
			}
		}
	}
;

compound_statement:
	'{' decl_or_stmt_list '}'	{
		ast_sym_t *temp;

		temp = $2;
		while (temp != NULL) {
			if (temp->stg_type == STG_NONE) {
				temp->stg_type = STG_AUTO_LOC;
			}
			temp = temp->prev;
		}

		$$ = $2;
	}
;

decl_or_stmt_list:
	%empty	{$$ = NULL;}
|	decl_or_stmt_list decl_or_stmt {
		if ($2 == NULL) {
			$$ = $1;
		} else {
			$2->prev = $1;
			$$ = $2;
		}
	}
;

decl_or_stmt:
	declaration {$$ = $1;}
|	statement {$$ = NULL;}
;

statement:
	compound_statement {
		insert_list(tab, $1, SCO_BLOCK, @1.last_line, 1);
		$$ = NULL;
	}
// |	expr ';' // not combined with parser yet
;

declaration:
	struct_type_def ';' {$$ = $1;}
|	STRUCT IDENT ';' {
		$$ = new_ast_sym($2, STG_NA, SYM_STRU_T, 
			new_ast_data(sizeof (int), DATA_STRU, QUAL_NONE,
				new_ast_stru(0, NULL, NULL)),
			strdup(@2.filename), @2.first_line);
		$$->data->node->stru->tag = $$;
	}
|	union_type_def ';' {$$ = $1;}
|	UNION IDENT ';' {
		$$ = new_ast_sym($2, STG_NA, SYM_UNIO_T, 
			new_ast_data(sizeof (int), DATA_UNIO, QUAL_NONE,
				new_ast_unio(0, NULL, NULL)),
			strdup(@2.filename), @2.first_line);
		$$->data->node->unio->tag = $$;
	}
|	enum_type_def ';' {$$ = $1;}
|	ENUM IDENT ';' {
		$$ = new_ast_sym($2, STG_NA, SYM_ENU_T, 
			new_ast_data(sizeof (int), DATA_ENU, QUAL_NONE,
				new_ast_enu(0, NULL)),
			strdup(@2.filename), @2.first_line);
		$$->data->node->enu->tag = $$;
	}
|	declaration_spec init_declarator_list ';' {
		ast_sym_t *curr;

		if ($1->data->data_type == DATA_NONE) {
			/*ERROR must specify a type*/
		}

		curr = $2;
		while (curr != NULL) {
			curr->stg_type = $1->stg_type;
			curr->tail = install_tail(curr, $1->data);
			if (curr->data->data_type == DATA_SCAL && curr->data->node->scal->scal_type == SCAL_VOID) {
				/*ERROR cannot have type void*/
			}
			curr = curr->prev;
		}

		// the below is wrong, declarator's sym info should be
		// at the END of the list; rework implementation
		if (list_start($1)->sym_type == SYM_NONE) {
			del_ast_sym($1);
		} else {
			list_start($$)->prev = $1;
		}

		$$ = $2;
	}
;

//
// declaration specifiers
//

declaration_spec:
	type_spec	{$$ = $1;}
|	type_spec untyped_declaration_spec	{
		$$ = $2;
		if ($$->data->data_type != DATA_NONE) {
			/*ERROR can only have one type specifier*/
		} else {
			$1->data->qual = $$->data->qual;
			del_ast_data($$->data);
			$$->data = copy_ast_data($1->data, -1);
			del_ast_sym($1);
		}
	}
|	stgclass_spec declaration_spec {
		$$ = $2;
		if ($$->stg_type != STG_NONE && $$->stg_type != $1) {
			/*ERROR*/
		} else {
			$$->stg_type = $1;
		}
	}
|	qual_spec declaration_spec {
		$$ = $2;
		if ($$->data == NULL) {
			$$->data = new_ast_data(0, DATA_NONE, $1, NULL);
		} else {
			$$->data->qual |= $1;
		}
	}
|	func_spec declaration_spec {
		$$ = $2;
		if ($$->sym_type != SYM_NONE && $$->sym_type != SYM_FUNC) {
			/*ERROR*/
		} else {
			$$->sym_type = SYM_FUNC;
		}
		$$->is_inline = 1;
	}
;

untyped_declaration_spec:
	stgclass_spec	{
		$$ = new_ast_sym(NULL, $1, SYM_NONE, NULL,
			strdup(@1.filename), @1.first_line);
		}
|	qual_spec	{
		$$ = new_ast_sym(NULL, STG_NONE, SYM_NONE,
			new_ast_data(0, DATA_NONE, $1, NULL),
			strdup(@1.filename), @1.first_line);
		}
|	func_spec	{
		$$ = new_ast_sym(NULL, STG_NONE, SYM_FUNC,
			new_ast_data(0, DATA_NONE, QUAL_NONE, NULL),
			strdup(@1.filename), @1.first_line);
		$$->is_inline = 1;
		}
|	stgclass_spec untyped_declaration_spec {
		$$ = $2;
		if ($$->stg_type != STG_NONE && $$->stg_type != $1) {
			/*ERROR*/
		} else {
			$$->stg_type = $1;
		}
	}
|	qual_spec untyped_declaration_spec {
		$$ = $2;
		if ($$->data == NULL) {
			$$->data = new_ast_data(0, DATA_NONE, $1, NULL);
		} else {
			$$->data->qual |= $1;
		}
	}
|	func_spec untyped_declaration_spec {
		$$ = $2;
		if ($$->sym_type != SYM_NONE && $$->sym_type != SYM_FUNC) {
			/*ERROR*/
		} else {
			$$->sym_type = SYM_FUNC;
		}
		$$->is_inline = 1;
	}
;
stgclass_spec:
	EXTERN	{$$ = (char) STG_EXTERN_EXP;}
|	STATIC	{$$ = (char) STG_STATIC;}
|	AUTO	{$$ = (char) STG_AUTO_LOC;}
|	REGISTER	{$$ = (char) STG_REGISTER;}
|	TYPEDEF	{$$ = (char) STG_TYPEDEF;}
;

qual_spec:
	CONST	{$$ = (char) QUAL_CONST;}
|	VOLATILE	{$$ = (char) QUAL_VOLATILE;}
|	RESTRICT	{$$ = (char) QUAL_RESTRICT;}
;

func_spec:
	INLINE
;

type_spec:
	float_type_spec	{
		$$ = new_ast_sym(NULL, STG_NONE, SYM_NONE, $1,
			strdup(@1.filename), @1.first_line);
	}
|	int_type_spec	{
		$$ = new_ast_sym(NULL, STG_NONE, SYM_NONE, $1,
			strdup(@1.filename), @1.first_line);
	}
|	enum_type_spec	{
		$$ = new_ast_sym(NULL, STG_NONE, SYM_NONE,
			copy_ast_data(list_start($1)->data, -1),
			strdup(@1.filename), @1.first_line);
		$$->prev = $1;
	}
|	struct_type_spec	{
		$$ = new_ast_sym(NULL, STG_NONE, SYM_NONE,
			copy_ast_data(list_start($1)->data, -1),
			strdup(@1.filename), @1.first_line);
		$$->prev = $1;
	}
|	union_type_spec	{
		$$ = new_ast_sym(NULL, STG_NONE, SYM_NONE,
			copy_ast_data(list_start($1)->data, -1),
			strdup(@1.filename), @1.first_line);
		$$->prev = $1;
	}
|	VOID	{
		$$ = new_ast_sym(NULL, STG_NONE, SYM_NONE,
			new_ast_data(0, DATA_SCAL, QUAL_NONE,
				new_ast_scal(0, SCAL_VOID)),
			strdup(@1.filename), @1.first_line);
	}
// |	IDENT {$$ = NULL; /*typedef not implemented*/}
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
	SHORT int_opt	{$$ = new_ast_data(sizeof (short), DATA_SCAL, QUAL_NONE, new_ast_scal(0, SCAL_SHORT));}
|	INT	{$$ = new_ast_data(sizeof (int), DATA_SCAL, QUAL_NONE, new_ast_scal(0, SCAL_INT));}
|	LONG int_opt	{$$ = new_ast_data(sizeof (long), DATA_SCAL, QUAL_NONE, new_ast_scal(0, SCAL_LONG));}
|	LONG LONG int_opt	{$$ = new_ast_data(sizeof (long long), DATA_SCAL, QUAL_NONE, new_ast_scal(0, SCAL_LONGLONG));}
|	SIGNED SHORT int_opt	{$$ = new_ast_data(sizeof (short), DATA_SCAL, QUAL_NONE, new_ast_scal(0, SCAL_SHORT));}
|	SIGNED	{$$ = new_ast_data(sizeof (int), DATA_SCAL, QUAL_NONE, new_ast_scal(0, SCAL_INT));}
|	SIGNED INT	{$$ = new_ast_data(sizeof (int), DATA_SCAL, QUAL_NONE, new_ast_scal(0, SCAL_INT));}
|	SIGNED LONG	int_opt	{$$ = new_ast_data(sizeof (long), DATA_SCAL, QUAL_NONE, new_ast_scal(0, SCAL_LONG));}
|	SIGNED LONG LONG int_opt	{$$ = new_ast_data(sizeof (long long), DATA_SCAL, QUAL_NONE, new_ast_scal(0, SCAL_LONGLONG));}
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

char_type_spec:
	CHAR	{$$ = new_ast_data(sizeof (char), DATA_SCAL, QUAL_NONE, new_ast_scal(0, SCAL_CHAR));}
|	SIGNED CHAR	{$$ = new_ast_data(sizeof (char), DATA_SCAL, QUAL_NONE, new_ast_scal(-1, SCAL_CHAR));}
|	UNSIGNED CHAR	{$$ = new_ast_data(sizeof (unsigned char), DATA_SCAL, QUAL_NONE, new_ast_scal(1, SCAL_CHAR));}
;

enum_type_spec:
	enum_type_def	{$$ = $1;}
|	enum_type_ref	{$$ = $1;}
;

enum_type_def:
	ENUM '{' enum_def_list comma_opt '}' {
		ast_sym_t *temp;
		$$ = new_ast_sym(NULL, STG_NA, SYM_ENU_T, 
			new_ast_data(sizeof (int), DATA_ENU, QUAL_NONE,
				new_ast_enu(1, NULL)),
			strdup(@1.filename), @1.first_line);
		$$->data->node->enu->tag = $$;

		temp = $3;
		while (temp != NULL) {
			temp->data = copy_ast_data($$->data, 1);
			temp = temp->prev;
		}
		$$ = $3;
	}
|	ENUM IDENT '{' enum_def_list comma_opt '}' {
		ast_sym_t *temp;
		$$ = new_ast_sym($2, STG_NA, SYM_ENU_T, 
			new_ast_data(sizeof (int), DATA_ENU, QUAL_NONE,
				new_ast_enu(1, NULL)),
			strdup(@2.filename), @2.first_line);
		$$->data->node->enu->tag = $$;

		temp = $4;
		while (temp != NULL) {
			temp->data = $$->data;
			temp = temp->prev;
		}
		list_start($4)->prev = $$;
		$$ = $4;
	}
;

enum_type_ref:
	ENUM IDENT	{
		$$ = new_ast_sym($2, STG_NA, SYM_ENU_T,
			new_ast_data(0, DATA_SUE, QUAL_NONE,
				new_ast_sue(DATA_ENU, strdup($2))),
			strdup(@2.filename), @2.first_line);
	}
;

enum_def_list:
	enum_constant_def	{
		$$ = new_ast_sym($1, STG_NONE, SYM_ENU_C, NULL,
			strdup(@1.filename), @1.first_line);
	}
|	enum_def_list ',' enum_constant_def	{
		$$ = new_ast_sym($3, STG_NONE, SYM_ENU_C, NULL,
			strdup(@3.filename), @3.first_line);
		$$->prev = $1;
	}
;

enum_constant_def:
	IDENT	{$$ = $1;}
// |	IDENT '=' expr // circle back when combining with parser
;


struct_type_spec:
	struct_type_def	{$$ = $1;}
|	struct_type_ref	{$$ = $1;}
;

struct_type_def:
	STRUCT '{' field_list '}' {
		$$ = new_ast_sym(NULL, STG_NA, SYM_STRU_T, 
			new_ast_data(0, DATA_STRU, QUAL_NONE,
				new_ast_stru(1, NULL, $3)),
			strdup(@1.filename), @1.first_line);
		$$->data->node->stru->tag = $$;

		struct_fix($$->data);
	}
|	STRUCT IDENT '{' field_list '}' {
		$$ = new_ast_sym($2, STG_NA, SYM_STRU_T, 
			new_ast_data(0, DATA_STRU, QUAL_NONE,
				new_ast_stru(1, NULL, $4)),
			strdup(@2.filename), @2.first_line);
		$$->data->node->stru->tag = $$;

		struct_fix($$->data);
	}
;

struct_type_ref:
	STRUCT IDENT	{
		$$ = new_ast_sym($2, STG_NA, SYM_STRU_T,
			new_ast_data(0, DATA_SUE, QUAL_NONE,
				new_ast_sue(DATA_STRU, strdup($2))),
			strdup(@2.filename), @2.first_line);
	}
;

union_type_spec:
	union_type_def	{$$ = $1;}
|	union_type_ref	{$$ = $1;}
;

union_type_def:
	UNION '{' field_list '}' {
		$$ = new_ast_sym(NULL, STG_NA, SYM_UNIO_T, 
			new_ast_data(0, DATA_UNIO, QUAL_NONE,
				new_ast_unio(1, NULL, $3)),
			strdup(@1.filename), @1.first_line);
		$$->data->node->unio->tag = $$;

		union_fix($$->data);
	}
|	UNION IDENT '{' field_list '}'  {
		$$ = new_ast_sym($2, STG_NA, SYM_UNIO_T, 
			new_ast_data(0, DATA_UNIO, QUAL_NONE,
				new_ast_unio(1, NULL, $4)),
			strdup(@2.filename), @2.first_line);
		$$->data->node->unio->tag = $$;

		union_fix($$->data);
	}
;

union_type_ref:
	UNION IDENT	{
		$$ = new_ast_sym($2, STG_NA, SYM_UNIO_T,
			new_ast_data(0, DATA_SUE, QUAL_NONE,
				new_ast_sue(DATA_UNIO, strdup($2))),
			strdup(@2.filename), @2.first_line);
	}
;

field_list:
	component_declaration {$$ = $1;}
|	field_list component_declaration {
		$$ = $2;
		list_start($$)->prev = $1;
	}
;

component_declaration:
	type_spec component_declarator_list ';' {
		ast_sym_t *temp = $2;
		while (temp != NULL) {
			temp->tail = install_tail(temp, copy_ast_data($1->data, -1));
			temp = temp->prev;
		}
		del_ast_sym($1);
		$$ = $2;
	}
;

component_declarator_list:
	component_declarator {$$ = $1;}
|	component_declarator_list ',' component_declarator {
		$$ = $3;
		list_start($$)->prev = $1;
	}
;

// bit fields not implemented, widths should support exprs
component_declarator:
	declarator {$$ = $1;}
|	':' NUM {
		$$ = new_ast_sym(NULL, STG_NA, SYM_NONE, NULL,
			strdup(@2.filename), @2.first_line);
	}
|	declarator ':' NUM {$$ = $1;}
;

// for casts and sizeof
// type_name:
// 	declaration_spec {
// 		$$ = $1->data;
// 		free($1);
// 	}
// |	declaration_spec abstract_declarator {
// 		install_tail($2, $1->data);
// 		free($1);
// 		$$ = $2;
// 	}
// ;

// declarators

init_declarator_list:
	init_declarator {$$ = $1;}
|	init_declarator_list ',' init_declarator {
		$$ = $3;
		list_start($$)->prev = $1;
	}
;

init_declarator:
	declarator {$$ = $1;}
// |	declarator '=' init {$$ = $1; /*circle back later, h&s 4.6. involves parser*/}
;

declarator:
	direct_declarator {$$ = $1;}
|	pointer direct_declarator {
		$2->tail = install_tail($2, $1);
		$$ = $2;
	}
;

pointer:
	'*' qual_spec_list	{
		$$ = new_ast_data(sizeof (void *), DATA_PTR, $2,
			new_ast_ptr(NULL));
	}
|	'*' qual_spec_list pointer	{
		$$ = new_ast_data(sizeof (void *), DATA_PTR, $2,
			new_ast_ptr($3));
	}
;

qual_spec_list:
	%empty {$$ = 0;}
|	qual_spec_list qual_spec {$$ |= $1;}
;

direct_declarator:
	IDENT	{
		$$ = new_ast_sym($1, STG_NONE, SYM_NONE, NULL,
			strdup(@1.filename), @1.first_line);
	}
|	'(' declarator ')'	{$$ = $2;}
|	array_declarator	{$$ = $1;}
|	function_declarator	{$$ = $1;}
;

// direct_decl[expr] not implemented here
array_declarator:
	direct_declarator '[' num_opt ']' {
		$$ = $1;
		$$->tail = install_tail($1,
			new_ast_data(0, DATA_ARY, QUAL_NONE,
				new_ast_ary($3, NULL)));
	}
;

function_declarator:
	direct_declarator '(' ')' {
		$$ = $1;
		$$->tail = install_tail($1,
			new_ast_data(0, DATA_FUNC, QUAL_NONE,
				new_ast_func(0, 0, NULL, NULL)));
	}
|	direct_declarator '(' param_list ')' {
		$$ = $1;
		$$->tail = install_tail($1,
			new_ast_data(0, DATA_FUNC, QUAL_NONE,
				new_ast_func(0, 0, NULL, $3)));
	}
|	direct_declarator '(' param_list ',' ELLIPSIS ')' {
		$$ = $1;
		$$->tail = install_tail($1,
			new_ast_data(0, DATA_FUNC, QUAL_NONE,
				new_ast_func(0, 1, NULL, $3)));
	}
// |	direct_declarator '(' ident_list ')' // k&r optional syntax
;

param_list:
	param_declaration {$$ = $1;}
|	param_list ',' param_declaration {
		$$ = $3;
		list_start($$)->prev = $1;
	}
;

param_declaration:
	declaration_spec param_declarator {
		$$ = $2;
		if ($1->stg_type == STG_REGISTER) {
			$$->stg_type = STG_REGISTER;
		} else {
			$$->stg_type = STG_AUTO_PAR;
			if ($1->stg_type != STG_NONE) {
				/*ERROR invalid stg class for parameter*/
			}
		}
		$$->sym_type = SYM_PARAM;
		$$->sco_type = SCO_PROTO;
		$$->tail = install_tail($$, $1->data);
		del_ast_sym($1);
	}
|	declaration_spec {
		$$ = $1;
		$$->stg_type = STG_AUTO_PAR;
		$$->sym_type = SYM_PARAM;
		$$->sco_type = SCO_PROTO;
	}
;

param_declarator:
	declarator	{$$ = $1;}
|	abstract_declarator	{$$ = $1;}
;

abstract_declarator:
	pointer {
		$$ = new_ast_sym(NULL, STG_NONE, SYM_NONE, $1,
			strdup(@1.filename), @1.first_line);
	}
|	pointer direct_abstract_declarator {
		$$ = $2;
		$$->tail = install_tail($$, $1);
	}
|	direct_abstract_declarator {$$ = $1;}
;

direct_abstract_declarator:
	'(' abstract_declarator ')' {$$ = $2;}
|	'[' num_opt ']' {
		$$ = new_ast_sym(NULL, STG_NONE, SYM_NONE,
			new_ast_data(0, DATA_ARY, QUAL_NONE,
				new_ast_ary($2, NULL)),
			strdup(@1.filename), @1.first_line);
	}
|	direct_abstract_declarator '[' num_opt ']'  {
		$1->tail = install_tail($1,
			new_ast_data(0, DATA_ARY, QUAL_NONE,
				new_ast_ary($3, NULL)));
		$$ = $1;
	}
|	'(' ')' {
		$$ = new_ast_sym(NULL, STG_NONE, SYM_NONE,
			new_ast_data(0, DATA_FUNC, QUAL_NONE,
				new_ast_func(0, 0, NULL, NULL)),
			strdup(@1.filename), @1.first_line);
	}
|	'(' param_list ')' {
		$$ = new_ast_sym(NULL, STG_NONE, SYM_NONE,
			new_ast_data(0, DATA_FUNC, QUAL_NONE,
				new_ast_func(0, 0, NULL, $2)),
			strdup(@1.filename), @1.first_line);
	}
|	direct_abstract_declarator '(' param_list ',' ELLIPSIS ')' {
		$1->tail = install_tail($1,
			new_ast_data(0, DATA_FUNC, QUAL_NONE,
				new_ast_func(0, 1, NULL, $3)));
		$$ = $1;
	}
;

// ident_list: // k&r optional syntax
// 	IDENT
// |	param_list ',' IDENT
// ;

num_opt:
	%empty {$$ = (TypedNumber) {{.i = 0}, TYPE_I};};
|	NUM {$$ = $1;}
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