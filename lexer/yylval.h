#ifndef YYLVAL_H
#define YYLVAL_H
enum numtypes {
    TYPE_I=0,
    TYPE_UI,
    TYPE_LI,
    TYPE_ULI,
    TYPE_LLI,
    TYPE_ULLI,
    TYPE_FD,
    TYPE_D,
    TYPE_LD
};

typedef union {
	long long int i;
	long double f;
} NumberValue;

typedef struct {
	NumberValue val;
	char type;
} TypedNumber;

typedef struct {
    char *li;
    int size;
} SizedString;

// typedef union {
// 	char *i;
// 	char c;
//     SizedString s;
// 	TypedNumber n;
// } YYSTYPE;

// extern YYSTYPE yylval;
// extern YYLTYPE yylloc;
// extern char *filename;
// extern int line;

#endif // YYLVAL_H