#include<stddef.h>
#include<stdio.h>
typedef struct lexer{
	const char *source;
	size_t position;
	size_t length;
	size_t line;
	size_t column;
}lexer;

typedef enum token_type{
	INVALID,
	UNTERM_COMMENT,
	UNTERM_CHAR,
	UNTERM_STRING,
	IDENTIFIER,
	NUMBER,
	STRING,
	CHAR,
	COMMENT,
	OPERATOR,
	DELIMITER,
	HASH,
	HASH_HASH,
	NEWLINE,
	SPACE,
	TOKEN_EOF,
}token_type;

typedef struct token{
	token_type type;
	size_t start;
	size_t length;
	size_t line;
	size_t column;
}token;


typedef struct macro_token{
	token_type type;
	char *text;
}macro_token;


typedef struct macro{
	int lookup_flag;
	char *name;
	macro_token *replacement;
	size_t replacement_count;
	struct macro *next;
}macro;


typedef struct macro_table{
	macro **buckets;
	size_t bucket_count;
	size_t macro_count;
}macro_table;


typedef struct preprocessor_context {
	macro_table macros;
	struct conditional *stack_top;
}preprocessor_context;


typedef struct conditional{
	int parent_active;
	int condition;
	int else_seen;
	struct conditional *next;
}conditional;


token lexer_next(lexer *);
char *token_type_print(token_type);
void print_macro_table(const macro_table *);

char *splice_remover(char *, size_t *);
char *tok_to_str(lexer *);
char *to_path(const char *, const char *);
int preprocessor(const char *, FILE *, FILE *, preprocessor_context *, unsigned int);
FILE *find_valid_path(const char *, const char *, char **);

int preprocessor_context_init(preprocessor_context *);
int skip_directive_line(lexer *, const char *, const char *, int);
int handle_directive(const char *file_name, lexer *lex, FILE *fd, preprocessor_context *ctx, unsigned int depth);
int handle_include(const char *file_name, lexer *lex, FILE *fd, preprocessor_context *ctx, unsigned int depth);
int handle_define(const char *, lexer *, macro_table *);
int handle_undef(const char *, lexer *, macro_table *);

int expand_macros(const token, const char *, const macro_table *, FILE *);
void expand_token(macro *, const macro_table *, FILE *);

int is_active(const preprocessor_context *);
void free_stack(preprocessor_context *);
int push_condition(preprocessor_context *, int);
void pop_condition(preprocessor_context *);
int handle_ifdef(const char *, lexer *, preprocessor_context *);
int handle_ifndef(const char *, lexer *, preprocessor_context *);
int handle_endif(const char *, lexer *, token, preprocessor_context *);
int handle_else(const char *, lexer *, token, preprocessor_context *);

size_t djb2(const char *);
size_t macro_hash(const char *, const size_t);
macro *macro_lookup(const macro_table *, const char *);
void free_replacement_tokens(macro *);
void insert_macro(macro_table *, macro *);
void macro_delete(macro_table *, const char *);
void macro_table_destroy(macro_table *);
