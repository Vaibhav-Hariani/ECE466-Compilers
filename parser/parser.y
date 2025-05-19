%{
     #include "parser.tab.h"
     int yylex(void);
     extern FILE *yyin;
	 char *filename;
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
%nonassoc ELSE

%union {
	char *i;
	char c;
	TypedNumber n;
    SizedString s;
    ast_node *node;
	ast_cpst_t *cpst;
    ast_data_t *data;
    ast_sym_t *sym;
    ast_tab_t *tab;
};

%code requires {
    #include <stdio.h>
	#include <string.h>
	#include <stdarg.h>
    #include "yylval.h"
    #include "util/expr.h"
	#include "util/stmt.h"
    #include "util/table.h"
    #include "util/symbol.h"
    #include "util/data.h"
    #include "out/expr_out.h"
    #include "out/stmt_out.h"
    #include "out/symtab_out.h"
    void yyerror(const char *format, ...);

	typedef struct ast_node ast_node;
	typedef struct ast_stmt ast_stmt_t;
	typedef struct ast_cpst ast_cpst_t;
	typedef struct ast_tab ast_tab_t;
	typedef struct ast_sym ast_sym_t;
	typedef struct ast_data ast_data_t;

	#define YYLTYPE YYLTYPE
	typedef struct YYLTYPE {
		int first_line;
		int first_column;
		int last_line;
		int last_column;
	} YYLTYPE;
};

%code {
	ast_tab_t *tab;
}

%initial-action {
	tab = new_table(0);
};

%define api.location.type {YYLTYPE}
%locations

%nterm <i> enum_constant_def;
%nterm <c> stgclass_spec qual_spec qual_spec_list;
%nterm <n> num_opt;
%nterm <data> float_type_spec int_type_spec;
%nterm <data> signed_type_spec unsigned_type_spec char_type_spec;
%nterm <data> pointer;
%nterm <data> type_name;
%nterm <cpst> statement compound_statement decl_or_stmt_list
%nterm <sym> prog declaration_or_fndef function_def;
%nterm <sym> declaration declaration_spec untyped_declaration_spec param_declaration;
%nterm <sym> init_declarator init_declarator_list;
%nterm <sym> type_spec;
%nterm <sym> enum_type_spec enum_type_def enum_type_ref enum_def_list;
%nterm <sym> struct_type_spec struct_type_def struct_type_ref;
%nterm <sym> union_type_spec union_type_def union_type_ref
%nterm <sym> component_declaration component_declarator component_declarator_list;
%nterm <sym> declarator;
%nterm <sym> direct_declarator array_declarator function_declarator;
%nterm <sym> param_declarator abstract_declarator direct_abstract_declarator;
%nterm <sym> field_list param_list;

%nterm <node> term_expr term_expr_opt expr binop_expr ternop_expr unop_expr assign_expr arg_list
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
	%empty	{$$ = NULL;}
|	prog declaration_or_fndef	{$$ = NULL;}
;

declaration_or_fndef:
	declaration {
		ast_sym_t *curr;

		curr = $1;
		while (curr != NULL) {
			switch (curr->stg_type) {
				case STG_EXTERN_EXP:
				case STG_STATIC:
					curr->stg_type = curr->stg_type;
					break;
				case STG_NONE:
					curr->stg_type = STG_EXTERN_IMP;
					break;
				default:
					/*ERROR invalid storage class specifier*/
					curr->stg_type = STG_EXTERN_IMP;
					break;
			}
			curr = curr->prev;
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
			install_tail($2, copy_ast_data($1->data, -1));
			del_ast_sym($1);

			if (!insert(tab, $2, SCO_FILE, __INT_MAX__, 1)) {	// insert function
				insert_list(tab, $2->data->node->func->params, SCO_FUNC,
					@3.last_line, 1);	// insert params
				insert_list(tab, $3->sym, SCO_FUNC,
					@3.last_line, 1);	// insert compound statement
				print_stmt_list($3->stmt);
				$$ = $2;
			} else {
				del_sym_list($3->sym);
				$$ = NULL;
			}
		}
	}
;

compound_statement:
	'{' decl_or_stmt_list '}'	{
		ast_sym_t *temp;

		temp = $2->sym;
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
	%empty	{$$ = new_ast_cpst(NULL, NULL);}
|	decl_or_stmt_list declaration {
		$$ = $1;
		if ($2 != NULL) {
			list_start($2)->prev = $$->sym;
			$$->sym = $2;
		}
	}
|	decl_or_stmt_list statement {
		$$ = $1;
		$$->stmt = append_stmt($$->stmt, $2->stmt);
		if ($2->sym != NULL) {
			list_start($2->sym)->prev = $$->sym;
			$$->sym = $2->sym;
		}
		free($2);
	}
;

statement:
	term_expr ';'	{
		$$ = new_ast_cpst(
			new_ast_stmt(new_ast_expr($1), STMT_EXPR, NULL),
			NULL);
	}
|	IF '(' term_expr ')' statement	{
		$$ = new_ast_cpst(
			new_ast_stmt(new_ast_if($3, $5->stmt, NULL), STMT_IF, NULL),
			NULL);
	}
|	IF '(' term_expr ')' statement ELSE statement	{
		$$ = new_ast_cpst(
			new_ast_stmt(new_ast_if($3, $5->stmt, $7->stmt), STMT_IF, NULL),
			NULL);
	}
|	FOR '(' term_expr_opt ';' term_expr_opt ';' term_expr_opt ')' statement	{
		$$ = new_ast_cpst(
			new_ast_stmt(new_ast_for($3, $5, $7, $9->stmt), STMT_IF, NULL),
			NULL);
	}
|	WHILE '(' term_expr ')' statement	{
		$$ = new_ast_cpst(
			new_ast_stmt(new_ast_while($3, $5->stmt), STMT_WHILE, NULL),
			NULL);
	}
|	DO statement WHILE '(' term_expr ')'	{
		$$ = new_ast_cpst(
			new_ast_stmt(new_ast_dowhile($5, $2->stmt), STMT_DOWHILE, NULL),
			NULL);
	}
|	CONTINUE ';'	{
		$$ = new_ast_cpst(
			new_ast_stmt(NULL, STMT_CONTINUE, NULL),
			NULL);
	}
|	BREAK ';'	{
		$$ = new_ast_cpst(
			new_ast_stmt(NULL, STMT_BREAK, NULL),
			NULL);
	}
|	SWITCH '(' term_expr ')' statement	{
		$$ = new_ast_cpst(
			new_ast_stmt(new_ast_switch($3, $5->stmt), STMT_SWITCH, NULL),
			NULL);
	}
|	GOTO IDENT ';'	{
		$$ = new_ast_cpst(
			new_ast_stmt(new_ast_goto(strdup($2)), STMT_GOTO, NULL),
			new_ast_sym(strdup($2), STG_NA, SYM_LABEL,
				new_ast_data(0, DATA_LABEL, QUAL_NONE, new_ast_label(0)),
				strdup(filename), @1.first_line));
	}
|	RETURN term_expr ';'	{
		$$ = new_ast_cpst(
			new_ast_stmt(new_ast_ret($2), STMT_RETURN, NULL),
			NULL);
	}
|	CASE term_expr ':' statement	{
		$$ = $4;
		$$->stmt = new_ast_stmt(new_ast_labelst(LABEL_CASE, $2, $4->stmt), STMT_CASE, NULL);
	}
|	DEFAULT ':' statement	{
		$$ = $3;
		$$->stmt = new_ast_stmt(new_ast_labelst(LABEL_DEFAULT, NULL, $3->stmt), STMT_DEFAULT, NULL);
	}
|	IDENT ':' statement {
		ast_sym_t *sym;

		$$ = $3;
		$$->stmt = new_ast_stmt(
			new_ast_labelst(LABEL_LABEL,
				new_ast_ident($1, strdup(filename), @1.first_line), $3->stmt),
			STMT_DEFAULT, NULL);
		sym = new_ast_sym(strdup($1), STG_NA, SYM_LABEL,
			new_ast_data(0, DATA_LABEL, QUAL_NONE, new_ast_label(1)),
			strdup(filename), @1.first_line);

		if ($$->sym == NULL) {
			$$->sym = sym;
		} else {
			list_start($$->sym)->prev = sym;
		}
	}
|	compound_statement	{
		ast_sym_t *labels, *prev, *curr, *next;

		curr = $1->sym;
		labels = NULL;
		next = NULL;
		
		// takes label symbols out of symbol list
		while (curr != NULL) {
			prev = curr->prev;
			if (curr->sym_type == SYM_LABEL) {
				if (next == NULL) {
					$1->sym = curr->prev;
				} else {
					next->prev = curr->prev;
				}

				curr->prev = NULL;
				if (labels == NULL) {
					labels = curr;
				} else {
					list_start(labels)->prev = curr;
				}
			} else {
				next = curr;
			}
			curr = prev;
		}

		// need to remove labels from here
		insert_list(tab, $1->sym, SCO_BLOCK, @1.last_line, 1);
		$1->sym = labels;
		$$ = new_ast_cpst(new_ast_stmt(new_ast_block($1), STMT_BLOCK, NULL), labels);
	}
;

term_expr_opt:
	%empty	{$$ = NULL;}
|	term_expr	{$$ = $1;}
;

declaration:
	struct_type_def ';' {$$ = $1;}
|	STRUCT IDENT ';' {
		$$ = new_ast_sym($2, STG_NA, SYM_STRU_T, 
			new_ast_data(sizeof (int), DATA_STRU, QUAL_NONE,
				new_ast_stru(0, NULL, NULL)),
			strdup(filename), @2.first_line);
		$$->data->node->stru->tag = $$;
	}
|	union_type_def ';' {$$ = $1;}
|	UNION IDENT ';' {
		$$ = new_ast_sym($2, STG_NA, SYM_UNIO_T, 
			new_ast_data(sizeof (int), DATA_UNIO, QUAL_NONE,
				new_ast_unio(0, NULL, NULL)),
			strdup(filename), @2.first_line);
		$$->data->node->unio->tag = $$;
	}
|	enum_type_def ';' {$$ = $1;}
|	ENUM IDENT ';' {
		$$ = new_ast_sym($2, STG_NA, SYM_ENU_T, 
			new_ast_data(sizeof (int), DATA_ENU, QUAL_NONE,
				new_ast_enu(0, NULL)),
			strdup(filename), @2.first_line);
		$$->data->node->enu->tag = $$;
	}
|	declaration_spec init_declarator_list ';' {
		ast_sym_t *curr;

		$$ = $2;
		if ($1->data->data_type == DATA_NONE) {
			fprintf(stderr, "%s:%d:Error: Must specify a scalar type.\n",
				filename, @2.first_line);
		}

		curr = $$;
		while (curr != NULL) {
			curr->stg_type = $1->stg_type;
			curr->sym_type = SYM_VAR;

			install_tail(curr, copy_ast_data($1->data, -1));

			if (curr->data->data_type == DATA_SCAL && curr->data->node->scal->scal_type == SCAL_VOID) {
				// inadequate, allows void arrays
				fprintf(stderr, "%s:%d:Error: Cannot have type void.\n",
					filename, @2.first_line);
			}
			curr = curr->prev;
		}

		if ($1->prev != NULL) {
			list_start($$)->prev = $1->prev;
		}
		del_ast_sym($1);
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
			strdup(filename), @1.first_line);
		}
|	qual_spec	{
		$$ = new_ast_sym(NULL, STG_NONE, SYM_NONE,
			new_ast_data(0, DATA_NONE, $1, NULL),
			strdup(filename), @1.first_line);
		}
|	func_spec	{
		$$ = new_ast_sym(NULL, STG_NONE, SYM_FUNC,
			new_ast_data(0, DATA_NONE, QUAL_NONE, NULL),
			strdup(filename), @1.first_line);
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
			strdup(filename), @1.first_line);
	}
|	int_type_spec	{
		$$ = new_ast_sym(NULL, STG_NONE, SYM_NONE, $1,
			strdup(filename), @1.first_line);
	}
|	enum_type_spec	{
		$$ = copy_ast_sym($1);
		$$->prev = $1;
		// $$ = new_ast_sym(NULL, STG_NONE, SYM_NONE,
		// 	copy_ast_data(list_start($1)->data, -1),
		// 	strdup(filename), @1.first_line);
		// $$->prev = $1;
	}
|	struct_type_spec	{
		$$ = copy_ast_sym($1);
		$$->prev = $1;
		// $$ = new_ast_sym(NULL, STG_NONE, SYM_NONE,
		// 	copy_ast_data(list_start($1)->data, -1),
		// 	strdup(filename), @1.first_line);
		// $$->prev = $1;
	}
|	union_type_spec	{
		$$ = copy_ast_sym($1);
		$$->prev = $1;
		// $$ = new_ast_sym(NULL, STG_NONE, SYM_NONE,
		// 	copy_ast_data(list_start($1)->data, -1),
		// 	strdup(filename), @1.first_line);
		// $$->prev = $1;
	}
|	VOID	{
		$$ = new_ast_sym(NULL, STG_NONE, SYM_NONE,
			new_ast_data(0, DATA_SCAL, QUAL_NONE,
				new_ast_scal(0, SCAL_VOID)),
			strdup(filename), @1.first_line);
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
			strdup(filename), @1.first_line);
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
			strdup(filename), @2.first_line);
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
				new_ast_sue(DATA_ENU, $2)),
			strdup(filename), @2.first_line);
	}
;

enum_def_list:
	enum_constant_def	{
		$$ = new_ast_sym($1, STG_NONE, SYM_ENU_C, NULL,
			strdup(filename), @1.first_line);
	}
|	enum_def_list ',' enum_constant_def	{
		$$ = new_ast_sym($3, STG_NONE, SYM_ENU_C, NULL,
			strdup(filename), @3.first_line);
		$$->prev = $1;
	}
;

enum_constant_def:
	IDENT	{$$ = $1;}
|	IDENT '=' term_expr // circle back when combining with parser
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
			strdup(filename), @1.first_line);
		$$->data->node->stru->tag = $$;

		struct_fix($$, $$->data);
	}
|	STRUCT IDENT '{' field_list '}' {
		$$ = new_ast_sym($2, STG_NA, SYM_STRU_T, 
			new_ast_data(0, DATA_STRU, QUAL_NONE,
				new_ast_stru(1, NULL, $4)),
			strdup(filename), @2.first_line);
		$$->data->node->stru->tag = $$;

		struct_fix($$, $$->data);
	}
;

struct_type_ref:
	STRUCT IDENT	{
		$$ = new_ast_sym($2, STG_NA, SYM_STRU_T,
			new_ast_data(0, DATA_SUE, QUAL_NONE,
				new_ast_sue(DATA_STRU, $2)),
			strdup(filename), @2.first_line);
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
			strdup(filename), @1.first_line);
		$$->data->node->unio->tag = $$;

		union_fix($$, $$->data);
	}
|	UNION IDENT '{' field_list '}'  {
		$$ = new_ast_sym($2, STG_NA, SYM_UNIO_T, 
			new_ast_data(0, DATA_UNIO, QUAL_NONE,
				new_ast_unio(1, NULL, $4)),
			strdup(filename), @2.first_line);
		$$->data->node->unio->tag = $$;

		union_fix($$, $$->data);
	}
;

union_type_ref:
	UNION IDENT	{
		$$ = new_ast_sym($2, STG_NA, SYM_UNIO_T,
			new_ast_data(0, DATA_SUE, QUAL_NONE,
				new_ast_sue(DATA_UNIO, $2)),
			strdup(filename), @2.first_line);
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
		$$ = $2;
		ast_sym_t *temp = $$;
		while (temp != NULL) {
			install_tail(temp, copy_ast_data($1->data, -1));
			temp = temp->prev;
		}

		if ($1->prev != NULL) {
			list_start($$)->prev = $1->prev;
		}
		// if ($1->data->data_type !=  DATA_STRU
		// && $1->data->data_type != DATA_UNIO
		// && $1->data->data_type != DATA_ENU) {
			del_ast_sym($1);
		// } else if ($1->name != NULL) {
		// 	list_start($$)->prev = $1;
		// }
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
			strdup(filename), @2.first_line);
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
		install_tail($2, $1);
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
	%empty {$$ = QUAL_NONE;}
|	qual_spec_list qual_spec {$$ |= $2;}
;

direct_declarator:
	IDENT	{
		$$ = new_ast_sym($1, STG_NONE, SYM_NONE, NULL,
			strdup(filename), @1.first_line);
	}
|	'(' declarator ')'	{$$ = $2;}
|	array_declarator	{$$ = $1;}
|	function_declarator	{$$ = $1;}
;

// direct_decl[expr] not implemented here
array_declarator:
	direct_declarator '[' num_opt ']' {
		$$ = $1;
		install_tail($1,
			new_ast_data(0, DATA_ARY, QUAL_NONE,
				new_ast_ary($3, NULL)));
	}
;

function_declarator:
	direct_declarator '(' ')' {
		$$ = $1;
		install_tail($1,
			new_ast_data(0, DATA_FUNC, QUAL_NONE,
				new_ast_func(0, 0, NULL, NULL)));
	}
|	direct_declarator '(' param_list ')' {
		$$ = $1;
		install_tail($1,
			new_ast_data(0, DATA_FUNC, QUAL_NONE,
				new_ast_func(0, 0, NULL, $3)));
	}
|	direct_declarator '(' param_list ',' ELLIPSIS ')' {
		$$ = $1;
		install_tail($1,
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
		install_tail($$, copy_ast_data($1->data, -1));
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
			strdup(filename), @1.first_line);
	}
|	pointer direct_abstract_declarator {
		$$ = $2;
		install_tail($$, $1);
	}
|	direct_abstract_declarator {$$ = $1;}
;

direct_abstract_declarator:
	'(' abstract_declarator ')' {$$ = $2;}
|	'[' num_opt ']' {
		$$ = new_ast_sym(NULL, STG_NONE, SYM_NONE,
			new_ast_data(0, DATA_ARY, QUAL_NONE,
				new_ast_ary($2, NULL)),
			strdup(filename), @1.first_line);
	}
|	direct_abstract_declarator '[' num_opt ']'  {
		install_tail($1,
			new_ast_data(0, DATA_ARY, QUAL_NONE,
				new_ast_ary($3, NULL)));
		$$ = $1;
	}
|	'(' ')' {
		$$ = new_ast_sym(NULL, STG_NONE, SYM_NONE,
			new_ast_data(0, DATA_FUNC, QUAL_NONE,
				new_ast_func(0, 0, NULL, NULL)),
			strdup(filename), @1.first_line);
	}
|	'(' param_list ')' {
		$$ = new_ast_sym(NULL, STG_NONE, SYM_NONE,
			new_ast_data(0, DATA_FUNC, QUAL_NONE,
				new_ast_func(0, 0, NULL, $2)),
			strdup(filename), @1.first_line);
	}
|	direct_abstract_declarator '(' param_list ',' ELLIPSIS ')' {
		install_tail($1,
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

//
// expressions
//

term_expr:
	assign_expr	{$$=$1;}
|	term_expr ',' assign_expr	{$$ = new_ast_double(AST_binop, $1, $3, ',', strdup(filename), @2.first_line);}

expr:
	NUM	{$$ = new_ast_num($1, strdup(filename), @1.first_line);}
|	IDENT	{$$ = new_ast_ident($1, strdup(filename), @1.first_line);}
|	CHARLIT	{$$ = new_ast_charlit($1, strdup(filename), @1.first_line);}
|	STRING	{$$ = new_ast_string($1, strdup(filename), @1.first_line);}
|	'(' term_expr ')'	{$$=$2;}
;

unop_expr:
	expr {$$=$1;}
|	unop_expr "++" %prec POSTFIX	{ $$ = new_ast_single($1, PLUSPLUS, POSTFIX, strdup(filename), @1.first_line);}
|	unop_expr "--" %prec POSTFIX	{ $$ = new_ast_single($1, MINUSMINUS, POSTFIX, strdup(filename), @1.first_line);}
|	"++" unop_expr %prec PREFIX	{ $$ = new_ast_single($2, PLUSPLUS, PREFIX, strdup(filename), @2.first_line);}
|	"--" unop_expr %prec PREFIX	{ $$ = new_ast_single($2, MINUSMINUS, PREFIX, strdup(filename), @2.first_line);}
|	unop_expr INDSEL IDENT	{ $$=new_ast_double(AST_binop, $1, new_ast_ident($3, strdup(filename), @3.first_line), INDSEL, strdup(filename), @1.first_line);}
|	unop_expr '.' IDENT	{ $$ = new_ast_double(AST_binop, $1, new_ast_ident($3, strdup(filename), @3.first_line), '.', strdup(filename), @1.first_line);}
//Helper function that should expand this into what it actually is
|	unop_expr '['term_expr']'	{ $$= ast_array_exp($1,$3, strdup(filename), @1.first_line);}
|	unop_expr '(' arg_list ')'	{ $$=new_ast_double(AST_funct, $1, $3, ')', strdup(filename), @1.first_line);}
|	'+' unop_expr %prec SIZEOF	{ $$ = new_ast_single($2, '+', PREFIX, strdup(filename), @2.first_line);}
|	'-' unop_expr %prec SIZEOF	{ $$= new_ast_single($2, '-', PREFIX, strdup(filename), @2.first_line);}
|	'!' unop_expr %prec SIZEOF	{ $$ = new_ast_single($2, '!', PREFIX, strdup(filename), @2.first_line);}
|	'~' unop_expr %prec SIZEOF	{ $$ = new_ast_single($2, '~', PREFIX, strdup(filename), @2.first_line);}
|	'&' unop_expr %prec SIZEOF	{ $$ = new_ast_single($2, '&', PREFIX, strdup(filename), @2.first_line);}
|	 '*' unop_expr %prec SIZEOF	{ $$ = new_ast_single($2, '*', PREFIX, strdup(filename), @2.first_line);}
|	SIZEOF unop_expr	{$$ = new_ast_single($2, SIZEOF, PREFIX, strdup(filename), @2.first_line);}
;

arg_list: %empty { $$ = new_ast_list(0);} 
|   assign_expr { $$=new_ast_list($1);}
|   arg_list ',' assign_expr %prec POSTFIX { $$ = append_ast_list($1, $3);}
;

binop_expr:
	unop_expr	{$$=$1;}
|	binop_expr '+' binop_expr	{ $$=new_ast_double(AST_binop, $1, $3, '+', strdup(filename), @1.first_line);}
|	binop_expr '-' binop_expr	{ $$=new_ast_double(AST_binop, $1, $3, '-', strdup(filename), @1.first_line);}
|	binop_expr '*' binop_expr	{ $$=new_ast_double(AST_binop, $1, $3, '*', strdup(filename), @1.first_line);}
|	binop_expr '/' binop_expr	{ $$=new_ast_double(AST_binop, $1, $3, '/', strdup(filename), @1.first_line);}
|	binop_expr '%' binop_expr	{ $$=new_ast_double(AST_binop, $1, $3, '%', strdup(filename), @1.first_line);}
|	binop_expr '>' binop_expr	{ $$=new_ast_double(AST_binop, $1, $3, '>', strdup(filename), @1.first_line);}
|	binop_expr '<' binop_expr	{ $$=new_ast_double(AST_binop, $1, $3, '<', strdup(filename), @1.first_line);}
|	binop_expr '&' binop_expr	{ $$=new_ast_double(AST_binop, $1, $3, '&', strdup(filename), @1.first_line);}
|	binop_expr '|' binop_expr	{ $$=new_ast_double(AST_binop, $1, $3, '|', strdup(filename), @1.first_line);}
|	binop_expr '^' binop_expr	{ $$=new_ast_double(AST_binop, $1, $3, '^', strdup(filename), @1.first_line);}
|	binop_expr SHL binop_expr	{ $$=new_ast_double(AST_binop, $1, $3, SHL, strdup(filename), @1.first_line);}
|	binop_expr SHR binop_expr	{ $$=new_ast_double(AST_binop, $1, $3, SHR, strdup(filename), @1.first_line);}
|	binop_expr EQEQ binop_expr	{ $$=new_ast_double(AST_binop, $1, $3, EQEQ, strdup(filename), @1.first_line);}
|	binop_expr NOTEQ binop_expr	{ $$=new_ast_double(AST_binop, $1, $3, NOTEQ, strdup(filename), @1.first_line);}
|	binop_expr LOGAND binop_expr	{ $$=new_ast_double(AST_binop, $1, $3, LOGAND, strdup(filename), @1.first_line);}
|	binop_expr LOGOR binop_expr	{ $$=new_ast_double(AST_binop, $1, $3, LOGOR, strdup(filename), @1.first_line);}
|	binop_expr LTEQ binop_expr	{ $$=new_ast_double(AST_binop, $1, $3, LTEQ, strdup(filename), @1.first_line);}
|	binop_expr GTEQ binop_expr	{ $$=new_ast_double(AST_binop, $1, $3, GTEQ, strdup(filename), @1.first_line);}

ternop_expr:
	binop_expr { $$=$1;}
|	binop_expr '?' binop_expr ':' binop_expr	{ $$=new_ast_ternop(AST_ternop, $1, $3, $5, strdup(filename), @1.first_line);};

assign_expr:
	ternop_expr	{$$=$1;}
|	unop_expr '=' assign_expr	{ $$=new_ast_double(AST_assign, $1, $3, '=', strdup(filename), @1.first_line);}
|	unop_expr TIMESEQ assign_expr	{ $$=new_ast_double(AST_assign, $1, $3, TIMESEQ, strdup(filename), @1.first_line);}
|	unop_expr DIVEQ assign_expr	{ $$=new_ast_double(AST_assign, $1, $3, DIVEQ, strdup(filename), @1.first_line);}
|	unop_expr MODEQ assign_expr	{ $$=new_ast_double(AST_assign, $1, $3, MODEQ, strdup(filename), @1.first_line);}
|	unop_expr PLUSEQ assign_expr	{ $$=new_ast_double(AST_assign, $1, $3, PLUSEQ, strdup(filename), @1.first_line);}
|	unop_expr MINUSEQ assign_expr	{ $$=new_ast_double(AST_assign, $1, $3, MINUSEQ, strdup(filename), @1.first_line);}
|	unop_expr SHLEQ assign_expr	{ $$=new_ast_double(AST_assign, $1, $3, SHLEQ, strdup(filename), @1.first_line);}
|	unop_expr SHREQ assign_expr	{ $$=new_ast_double(AST_assign, $1, $3, SHLEQ, strdup(filename), @1.first_line);}
|	unop_expr ANDEQ assign_expr	{ $$=new_ast_double(AST_assign, $1, $3, ANDEQ, strdup(filename), @1.first_line);}
|	unop_expr OREQ assign_expr	{ $$=new_ast_double(AST_assign, $1, $3, OREQ, strdup(filename), @1.first_line);}
|	unop_expr XOREQ assign_expr	{ $$=new_ast_double(AST_assign, $1, $3, XOREQ, strdup(filename), @1.first_line);}
;

%%

/* #ifdef YYDEBUG
    extern int yydebug = 1;
#endif */


void yyerror(const char *format, ...){
	va_list args;
	va_start(args, format);
	fprintf(stderr, "Error: ");
    vfprintf(stderr, format, args);
	va_end(args);
}

int main(int argc, char** argv){
    FILE *file;
    if(argc < 2) {
        /* yyerror("No File Specified");
        return 0; */
        yyin = stdin;
		filename = strdup("stdin");
        fprintf(stderr, "No File Specified \n");
    } else {
        file = fopen(argv[1],"r");
		filename = strdup(argv[1]);
        if(!file) {
            yyerror("No valid file specified");
            return 0;
        }
        yyin = file;
    }
	fprintf(stderr, "\n");
    yyparse();
    if(yyin != stdin) {
        fclose(file);    
    }

    return 0;
    /* yyparse();  */
} 