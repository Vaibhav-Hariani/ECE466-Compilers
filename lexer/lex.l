%option noyywrap
%option nounput
%option noinput

%{
#include "symtab.tab.h"
#include "yylval.h"
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>

// YYSTYPE yylval;
// YYLTYPE yylloc;
extern char *filename;
int line = 1;

void new_filename();
void char_lit();
void str_lit();
int literal_val(char *buf, int is_str);
void number_val(char type);

#define YY_USER_ACTION yylloc.first_line = yylloc.last_line = line;
%}

id			[A-Za-z_][A-Za-z0-9_]*

intsuf		(([Uu]?((L?L?)|(l?l?)))|(((L?L?)|(l?l?))[Uu]?))
octint		0[0-7]*{intsuf}
decint		[1-9][0-9]*{intsuf}
hexint		0[Xx][0-9A-Fa-f]+{intsuf}
deci2f		[0-9]+([Ee][\+-]?[0-9]+)[FLfl]?
hexi2f		0[Xx][0-9A-Fa-f]+([Pp][\+-]?[0-9]+)[FLfl]?
decflt		(([0-9]+\.[0-9]*)|([0-9]*\.[0-9]+))([Ee][\+-]?[0-9]+)?[FLfl]?
hexflt		0[Xx](([0-9A-Fa-f]+\.[0-9A-Fa-f]*)|([0-9A-Fa-f]*\.[0-9A-Fa-f]+))([Pp][\+-]?[0-9]+)?[FLfl]?

/* from here until str_lit defines: */
/*  char_lit (ref C99 w.d. 6.4.4.4) */
/*  str_lit (ref C99 w.d. 6.5) */
simple_esc	\\[\'\"\?\\abfnrtv]
oct_esc		\\[0-7]?[0-7]?[0-7]?
hex_esc		\\x[0-9A-Fa-f]+
hex_quad	[0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f]
uni_char	\\(u|(U{hex_quad})){hex_quad}
esc_seq		{simple_esc}|{oct_esc}|{hex_esc}|{uni_char}

c_char		[^\'\\\n]|{esc_seq}
char_lit	L?\'{c_char}+\'

s_char		[^\"\\\n]|{esc_seq}
str_lit		L?\"{s_char}*\"

preproc		"# "[0-9]+" \""(.+)\".*\n
newline		\n
ws			[ \t\r]+

/* comments are removed by preprocessing */
/* this is just useful during debugging */
cmtlin		"//".*\n

%%
";"				{return(';');}

"+"				{return('+');}
"-"				{return('-');}
"*"				{return('*');}
"/"				{return('/');}
"%"				{return('%');}
">"				{return('>');}
"<"				{return('<');}
"="				{return('=');}

"."				{return('.');}
","				{return(',');}

":"				{return(':');}
"?"				{return('?');}
"&"				{return('&');}
"|"				{return('|');}
"!"				{return('!');}
"^"				{return('^');}
"~"				{return('~');}

"("				{return('(');}
")"				{return(')');}
"["				{return('[');}
"]"				{return(']');}
"{"				{return('{');}
"}"				{return('}');}

"<:"			{return('[');}
":>"			{return(']');}
"<%"			{return('{');}
"%>"			{return('}');}
"%:"			{return('#');}
"%:%:"			{return(POUNDPOUND);}

"->"			{return(INDSEL);}
"++"			{return(PLUSPLUS);}
"--"			{return(MINUSMINUS);}
"<<"			{return(SHL);}
">>"			{return(SHR);}
"<="			{return(LTEQ);}
">="			{return(GTEQ);}
"=="			{return(EQEQ);}
"!="			{return(NOTEQ);}
"&&"			{return(LOGAND);}
"||"			{return(LOGOR);}
"..."			{return(ELLIPSIS);}
"*="			{return(TIMESEQ);}
"/="			{return(DIVEQ);}
"%="			{return(MODEQ);}
"+="			{return(PLUSEQ);}
"-="			{return(MINUSEQ);}
"<<="			{return(SHLEQ);}
">>="			{return(SHREQ);}
"&="			{return(ANDEQ);}
"|="			{return(OREQ);}
"^="			{return(XOREQ);}
"##"			{return(POUNDPOUND);}

"auto"			{return(AUTO);}
"break"			{return(BREAK);}
"case"			{return(CASE);}
"char"			{return(CHAR);}
"const"			{return(CONST);}
"continue"		{return(CONTINUE);}
"default"		{return(DEFAULT);}
"do"			{return(DO);}
"double"		{return(DOUBLE);}
"else"			{return(ELSE);}
"enum"			{return(ENUM);}
"extern"		{return(EXTERN);}
"float"			{return(FLOAT);}
"for"			{return(FOR);}
"goto"			{return(GOTO);}
"if"			{return(IF);}
"inline"		{return(INLINE);}
"int"			{return(INT);}
"long"			{return(LONG);}
"register"		{return(REGISTER);}
"restrict"		{return(RESTRICT);}
"return"		{return(RETURN);}
"short"			{return(SHORT);}
"signed"		{return(SIGNED);}
"sizeof"		{return(SIZEOF);}
"static"		{return(STATIC);}
"struct"		{return(STRUCT);}
"switch"		{return(SWITCH);}
"typedef"		{return(TYPEDEF);}
"union"			{return(UNION);}
"unsigned"		{return(UNSIGNED);}
"void"			{return(VOID);}
"volatile"		{return(VOLATILE);}
"while"			{return(WHILE);}
"_Bool"			{return(BOOL);}
"_Complex"		{return(COMPLEX);}
"_Imaginary"	{return(IMAGINARY);}

{id}			{yylval.i=strdup(yytext);	return(IDENT);}
{char_lit}		{char_lit();				return(CHARLIT);}
{str_lit}		{str_lit();					return(STRING);}
{octint}		{number_val('o');			return(NUM);}
{decint}		{number_val('i');			return(NUM);}
{hexint}		{number_val('h');			return(NUM);}
{deci2f}		{number_val('I');			return(NUM);}
{hexi2f}		{number_val('H');			return(NUM);}
{decflt}		{number_val('I');			return(NUM);}
{hexflt}		{number_val('H');			return(NUM);}

{preproc}		{/*new_filename();*/ line=1;}
{newline}		{line++;}
{cmtlin}		{line++;}
{ws}			{/*empty*/}
.				{fprintf(stderr, "%s:%d:Error:Unrecognised token %s\n", filename, line, yytext);}

%%

void new_filename() {
	free(filename);
	filename = strndup(yytext+5, strlen(yytext)-7);
	fprintf(stderr, "new filename is %s\n", filename);
}

void char_lit() {
	literal_val(&yylval.c, 0);
}

void str_lit() {
	char *buf;
	SizedString s;

	buf = (char *) calloc(yyleng-2, sizeof(char));
	s.size = literal_val(buf, 1);
	s.li = (char *) realloc(buf, s.size);
	yylval.s = s;
}

/*  convert all escape sequences
	to characters in buf */
int literal_val(char *buf, int is_str) {
	int i, char_done, buf_idx, num_char;
	char *num_esc, *num_end;

	char_done = buf_idx = 0;

	/* exclude '' and "" */
	for (i = 1 + (yytext[0] == 'L'); i < yyleng-1 && !char_done; i++) {
		if (yytext[i] == '\\') {
			/* esc seq start*/
			switch (yytext[++i]) { /* first char of esc seq */
				case '\'': case '\"':
				case '\\': case '\?':
					buf[buf_idx++] = yytext[i];
					break;
				case 'a':
					buf[buf_idx++] = '\a';
					break;
				case 'b':
					buf[buf_idx++] = '\b';
					break;
				case 'f':
					buf[buf_idx++] = '\f';
					break;
				case 'n':
					buf[buf_idx++] = '\n';
					break;
				case 'r':
					buf[buf_idx++] = '\r';
					break;
				case 't':
					buf[buf_idx++] = '\t';
					break;
				case 'v':
					buf[buf_idx++] = '\v';
					break;
				case 'u': case 'U':
					/* optional, unimplemented */
					break;
				case 'x': /* hex esc sequence */
					num_esc = yytext + i + 1;
					num_char = strtol(num_esc, &num_end, 16);
					i += (num_end - num_esc) / sizeof(char);

					if (num_char > 255) { // restrict overflow
						fprintf(stderr, "%s:%d:Warning:Hex escape sequence \\x%x out of range\n",
							filename, line, num_char);
						num_char = 255;
					}
					buf[buf_idx++] = (char) num_char;
					break;
				default: /* octal esc sequence */
					num_esc = (char *) calloc(3, sizeof(char));
					strncpy(num_esc, yytext + i, 3);
					num_char = strtol(num_esc, &num_end, 8);
					i += (num_end == 0)? 2 : (num_end - num_esc) / sizeof(char) - 1;
					free(num_esc);

					if (num_char > 255) { // restrict overflow
						fprintf(stderr, "%s:%d:Warning:Oct escape sequence \\o%o out of range\n",
							filename, line, num_char);
						num_char = 255;
					}
					buf[buf_idx++] = (char) num_char;
					break;
			}
		} else { /* not in esc seq */
			buf[buf_idx++] = yytext[i];
		}
		char_done = !is_str;
	}

	if (char_done && yytext[i] != '\'') {
		fprintf(stderr,
			"%s:%d:Warning:Unsupported multibyte character literal truncated to first byte\n",
			filename, line);
	}
	return buf_idx;
}

/* determine the type of number,
   store in TypedNumber number */
void number_val(char type) {
	TypedNumber tn;


	if (type == 'o' || type == 'i' || type == 'h') { /* integer */
		tn.type = TYPE_I;

		for (int idx = yyleng-1; idx >= yyleng-4 && idx >= 0; idx--) {

			if (yytext[idx] == 'U' || yytext[idx] == 'u') {
				tn.type++;
			} else if (yytext[idx] == 'L' || yytext[idx] == 'l') {
				tn.type += 2;
			} else {
				break;
			}
		}

		errno = 0;
		tn.val.i = strtoull(yytext, NULL, 0);
		
		if (errno == ERANGE) {/* ERROR */
			fprintf(stderr,
				"%s:%d:Warning:Integer constant exceeds valid range\n",
				filename, line);
		}
	}	
	else { /* floating */
		tn.type = TYPE_D;
		tn.type += (yytext[yyleng-1] == 'L' || yytext[yyleng-1] == 'l');
		tn.type -= (yytext[yyleng-1] == 'F' || yytext[yyleng-1] == 'f');

		errno = 0;
		tn.val.f = strtold(yytext, NULL);

		if (errno == ERANGE) {/* ERROR */
			fprintf(stderr,
				"%s:%d:Warning:Floating constant exceeds valid range\n",
				filename, line);
		}
	}
	yylval.n = tn;
}
