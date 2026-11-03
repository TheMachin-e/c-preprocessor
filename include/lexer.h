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


typedef struct preprocesso_context {
	macro_table macros;
}preprocessor_context;


token lexer_next(lexer *);
//char *token_type_print(token_type);
char *splice_remover(char *, size_t *);
char *tok_to_str(lexer *);
char *to_path(const char *, const char *);
int preprocessor(const char *, FILE *, FILE *, unsigned int);
FILE *find_valid_path(const char *, const char *, char **);
int preprocessor_context_init(preprocessor_context *);
void free_preprocessor_context(preprocessor_context *);
size_t djb2(const char *);
size_t macro_hash(char *, const size_t);
macro *macro_lookup(const macro_table *, const char *);
void macro_delete(macro_table *, const char *);
void macro_table_destroy(macro_table *);
