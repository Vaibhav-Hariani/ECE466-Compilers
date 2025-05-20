#ifndef TABLE_H
#define TABLE_H

#include "symbol.h"
#include "data.h"

typedef struct ast_sym ast_sym_t;
typedef struct ast_data ast_data_t;

enum ns_type {
    NS_MISC = 0,
    NS_TAG,
    NS_MEMB,
    NS_LABEL
};

typedef struct ast_cell {
    char is_deleted;
    ast_sym_t *sym;
} ast_cell_t;

typedef struct ast_tab {
    unsigned int size;
    unsigned int filled;
    ast_cell_t **cells;
} ast_tab_t;


// Returns the namespace to which sym_type belongs.
int get_namespace(short sym_type);

// Initialises a hash table. The default size for the
// internal cell array is the smallest size provided
// by sizeup() that is greater than or equal to min_size.
ast_tab_t *new_table(unsigned int min_size);

// Insert sym into tab. If replace_dup is 0, an error occurs
// if a symbol with the same name and namespace exists;
// otherwise, such a symbol is replaced if replacement is
// permissible. Returns 0 on success, 1 if a symbol in the
// same namespace and scope already exists in the table, 2
// if rehash fails. Sets the end of sym's scope to end and
// its scope types to sco_type.
int insert(ast_tab_t *tab, ast_sym_t *sym, char sco_type, int end, char replace_dup);

// Insert list of symbols (the last of which is pointed to
// by sym) into tab. If replace_dup is 0, an error
// occurs if a symbol with the same name and namespace
// exists; otherwise, such a symbol is replaced if
// replacement is permissible. Returns the number of symbols
// not entered into a table (0 if all successfully entered).
// Sets the end of each symbol's scope to end and their
// scope types to sco_type.
int insert_list(ast_tab_t *tab, ast_sym_t *sym, char sco_type, int end, char replace_dup);

// Returns true if a symbol with the specified key (name),
// namespace and whose scope contains start and end is in
// the hash table, false otherwise.
int contains(ast_tab_t *tab, char *key, char namespace, int start, int end);

// Returns a pointer to the symbol with the specified key
// (name), namespace and whose scope is the smallest scope
// which contains start and end for that key and namespace
// if it exists, NULL otherwise.
ast_sym_t *get_sym(ast_tab_t *tab, char *key, char namespace, int start, int end);

// Overwrites the symbol whose key and namespace match those
// of sym and whose scope is the smallest scope which
// contains sym->start and sym->end with sym. Deletes the
// old symbol.
int set_sym(ast_tab_t *tab, ast_sym_t *sym);

// Deletes the symbol whose key and namespace match those
// passed and whose scope is the smallest scope which
// contains start and end with sym. Returns 0 on success,
// -1 if no such symbol existed.
int remove_sym(ast_tab_t *tab, char *key, char namespace, int start, int end);

// Deletes the symbol table and all that it contains.
int del_table(ast_tab_t *tab);

// Returns a prime number of magnitude greater than or equal
// to the size passed, or -1 if that would be too big.
int sizeup(unsigned int size);

// A simple hash function based on a symbol's name (key) and
// its namespace.
unsigned int hash(ast_tab_t *tab, char *key, char namespace);

// Returns -1 if a symbol with the specified key and
// namespace and whose scope is or surrounds that defined
// by start and end does not exist, or the cell array index
// of the symbol whose scope most closely fits start and end
// if otherwise.
unsigned int locate(ast_tab_t *tab, char *key, char namespace, int start, int end);

// Increases the size of our table to the next size returned
// by sizeup and reinserts only the non-deleted cells.
int rehash(ast_tab_t *tab);


#endif // TABLE_H