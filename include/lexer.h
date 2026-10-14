#include<stddef.h>
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
	TOKEN_EOF,
}token_type;

typedef struct token{
	token_type type;
	size_t start;
	size_t length;
	size_t line;
	size_t column;
}token;



token lexer_next(lexer *);
char *token_type_print(token_type);
char *splice_remover(char *, int);
