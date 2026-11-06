#include<stdlib.h>
#include<lexer.h>
#include<string.h>

int is_active(const preprocessor_context *ctx){
	if(!ctx->stack_top){
		return 1;
	}
	conditional *top = ctx->stack_top;
	return top->parent_active && (top->else_seen ? !top->condition : top->condition);
}

void free_stack(preprocessor_context *ctx){
	conditional *next, *node = ctx->stack_top;
	while(node){
		next = node->next;
		free(node);
		node = next;
	}
	ctx->stack_top = NULL;
}

int push_condition(preprocessor_context *ctx, int condition){
	conditional *new  = calloc(1, sizeof(conditional));
	if(!new){
		perror("calloc");
		return 1;
	}	
	new->parent_active = is_active(ctx);
	new->condition = condition;
	new->next = ctx->stack_top;
	ctx->stack_top = new;
	return 0;
}


void pop_condition(preprocessor_context *ctx){
	conditional *node = ctx->stack_top;
	if(!node)
		return;
	
	ctx->stack_top = node->next;
	free(node);
}


int handle_ifdef(const char *file_name, lexer *lex, preprocessor_context *ctx){
	token tok;
	do{
		tok = lexer_next(lex);
	}while(tok.type == SPACE || tok.type == COMMENT);

	if(tok.type == NEWLINE || tok.type == TOKEN_EOF){
		fprintf(stderr, "%s:%zu:%zu: \033[1;31merror\033[0m: no macro name given with \033[1m'#ifdef'\033[0m directive\n", file_name, tok.line, tok.column);
		return 1;
	}    
	else if(tok.type != IDENTIFIER){
		fprintf(stderr, "%s:%zu:%zu: \033[1;31merror\033[0m: macro names must be identifier\n", file_name, tok.line, tok.column);
		return 1;
	}
	
	char *identifier = malloc(tok.length + 1);
	if(!identifier){
		perror("malloc");
		return 1;
	}
	
	memcpy(identifier, lex->source + tok.start, tok.length);
	identifier[tok.length] = '\0';
	
	int condition = 0;	
	if(is_active(ctx)) // macro lookup only if parent is active else it is redundant operation
		if(macro_lookup(&ctx->macros, identifier))
			condition = 1;
		
	free(identifier);

	int res;
	res = push_condition(ctx, condition);
	if(res)
		return res;


	int flag = 0;
	while((tok = lexer_next(lex)).type != NEWLINE && tok.type != TOKEN_EOF){
                if(tok.type != SPACE && tok.type != COMMENT && !flag){
                        fprintf(stderr, "%s:%zu:%zu: \033[1;35mwarning\033[0m: extra tokens at the end of \033[1m'#undef'\033[0m directive\n", file_name, tok.line, tok.column);
                        flag = 1;
                }
                if(tok.type == UNTERM_COMMENT || tok.type == UNTERM_STRING || tok.type == UNTERM_CHAR){
                        fprintf(stderr, "%s:%zu:%zu: \033[1;31merror\033[0m: unterminated token at the end of \033[1m'#undef'\033[0m directive\n", file_name, tok.line, tok.column);
                        return 1;
                }
        }
	
	return 0;
}


int handle_ifndef(const char *file_name, lexer *lex, preprocessor_context *ctx){
	token tok;
	do{
		tok = lexer_next(lex);
	}while(tok.type == SPACE || tok.type == COMMENT);

	if(tok.type == NEWLINE || tok.type == TOKEN_EOF){
		fprintf(stderr, "%s:%zu:%zu: \033[1;31merror\033[0m: no macro name given with \033[1m'#ifdef'\033[0m directive\n", file_name, tok.line, tok.column);
		return 1;
	}    
	else if(tok.type != IDENTIFIER){
		fprintf(stderr, "%s:%zu:%zu: \033[1;31merror\033[0m: macro names must be identifier\n", file_name, tok.line, tok.column);
		return 1;
	}
	
	char *identifier = malloc(tok.length + 1);
	if(!identifier){
		perror("malloc");
		return 1;
	}
	
	memcpy(identifier, lex->source + tok.start, tok.length);
	identifier[tok.length] = '\0';
	
	int condition = 0;	
	if(is_active(ctx))
		condition = !macro_lookup(&ctx->macros, identifier);

	free(identifier);	

	int res;
	res = push_condition(ctx, condition);
	if(res)
		return res;

	int flag = 0;
	while((tok = lexer_next(lex)).type != NEWLINE && tok.type != TOKEN_EOF){
                if(tok.type != SPACE && tok.type != COMMENT && !flag){
                        fprintf(stderr, "%s:%zu:%zu: \033[1;35mwarning\033[0m: extra tokens at the end of \033[1m'#ifndef'\033[0m directive\n", file_name, tok.line, tok.column);
                        flag = 1;
                }
                if(tok.type == UNTERM_COMMENT || tok.type == UNTERM_STRING || tok.type == UNTERM_CHAR){
                        fprintf(stderr, "%s:%zu:%zu: \033[1;31merror\033[0m: unterminated token at the end of \033[1m'#ifndef'\033[0m directive\n", file_name, tok.line, tok.column);
                        return 1;
                }
        }
	
	return 0;
}

int handle_endif(const char *file_name, lexer *lex, token tok, preprocessor_context *ctx){
	if(!ctx->stack_top){
		fprintf(stderr, "%s:%zu:%zu: \033[1;31merror\033[0m: \033[1m'#endif'\033[0m without \033[1m'#if'\033[0m directive\n", file_name, tok.line, tok.column);
                return 1;
	}
	
	pop_condition(ctx);

	int flag = 0;
	while((tok = lexer_next(lex)).type != NEWLINE && tok.type != TOKEN_EOF){
                if(tok.type != SPACE && tok.type != COMMENT && !flag){
                        fprintf(stderr, "%s:%zu:%zu: \033[1;35mwarning\033[0m: extra tokens at the end of \033[1m'#endif'\033[0m directive\n", file_name, tok.line, tok.column);
                        flag = 1;
                }
                if(tok.type == UNTERM_COMMENT || tok.type == UNTERM_STRING || tok.type == UNTERM_CHAR){
                        fprintf(stderr, "%s:%zu:%zu: \033[1;31merror\033[0m: unterminated token at the end of \033[1m'#endif'\033[0m directive\n", file_name, tok.line, tok.column);
                        return 1;
                }
        }
	
	return 0;
	
}

int handle_else(const char *file_name, lexer *lex, token tok, preprocessor_context *ctx){
	conditional *top = ctx->stack_top;
	if(!top){
		fprintf(stderr, "%s:%zu:%zu: \033[1;31merror\033[0m: \033[1m'#else'\033[0m without \033[1m'#if'\033[0m\n", file_name, tok.line, tok.column);
     		return 1;
   	}

	if(top->else_seen){
		fprintf(stderr, "%s:%zu:%zu: \033[1;31merror\033[0m: \033[1m'#else'\033[0m after \033[1m'#else'\033[0m\n", file_name, tok.line, tok.column);
     		return 1;
	}

	top->else_seen = 1;
	
	int flag = 0;
	while((tok = lexer_next(lex)).type != NEWLINE && tok.type != TOKEN_EOF){
                if(tok.type != SPACE && tok.type != COMMENT && !flag){
                        fprintf(stderr, "%s:%zu:%zu: \033[1;35mwarning\033[0m: extra tokens at the end of \033[1m'#else'\033[0m directive\n", file_name, tok.line, tok.column);
                        flag = 1;
                }
                if(tok.type == UNTERM_COMMENT || tok.type == UNTERM_STRING || tok.type == UNTERM_CHAR){
                        fprintf(stderr, "%s:%zu:%zu: \033[1;31merror\033[0m: unterminated token at the end of \033[1m'#else'\033[0m directive\n", file_name, tok.line, tok.column);
                        return 1;
                }
        }
	
	return 0;
}
