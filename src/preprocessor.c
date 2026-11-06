#include<stdlib.h>
#include<string.h>
#include"lexer.h"
#define MAX_INCLUDE_DEPTH 100
#define INITIAL_BUCKET_COUNT 16


int preprocessor_context_init(preprocessor_context *ctx){
	ctx->macros.bucket_count = INITIAL_BUCKET_COUNT;
	ctx->macros.macro_count = 0;
	ctx->macros.buckets = calloc(ctx->macros.bucket_count, sizeof(macro *));
	if(!ctx->macros.buckets){
		perror("calloc");
		return 1;
	}
	return 0;
}


int preprocessor(const char *file_name, FILE *fp, FILE *fd, preprocessor_context *ctx,unsigned int depth){
	if(depth >= MAX_INCLUDE_DEPTH){
		fprintf(stderr, "%s: #include nested depth exceeded 100\n", file_name);
		return 1;
	}
	if(fseek(fp, 0, SEEK_END) != 0){
		perror("fseek");
		return 1;
	}
	long file_len = ftell(fp);
	if(file_len < 0){
		perror("ftell");
		return 1;
	}
	size_t source_len = (size_t)file_len + 1;
	rewind(fp);
	char *row_source = malloc(sizeof(char) * source_len);
	if(!row_source){
		perror("malloc");
		return 1;
	}
	if(fread(row_source, 1, (size_t)file_len, fp) != (size_t)file_len){
		fprintf(stderr, "%s: Failed to read input file\n", file_name);
		free(row_source);
		return 1;
	}
	row_source[file_len] = '\0';
	char *source = splice_remover(row_source, &source_len);
	if(!source){
		perror("malloc");
		free(row_source);
		row_source = NULL;
		return 1;	
	}
	char *header_file_name;
	char *file_path;
	lexer lex = {source, 0, source_len, 1, 1};
	token tok = {0};
	int at_line_start = 1;
	int res;
	FILE * fs;
	while((tok = lexer_next(&lex)).type != TOKEN_EOF){
	//	fprintf(stdout, "%.*s : %s \n", (int)tok.length, source + tok.start, token_type_print(tok.type));
		if(tok.type == HASH && at_line_start){
			res = handle_directive(file_name, &lex, fd, ctx, depth);
			if(res != 0){
				free(source);
				free(row_source);
				return res;
			}
			continue;
		}
		switch(tok.type){
			case UNTERM_COMMENT:
				fprintf(stderr, "%s:%zu:%zu: error: unterminated comment\n", file_name, tok.line, tok.column);
				free(source);
				free(row_source);
				return 1;
			case UNTERM_CHAR:
				fprintf(stderr, "%s:%zu:%zu: error: missing terminating ' character\n", file_name, tok.line, tok.column);
				free(source);
				free(row_source);
				break;
			case UNTERM_STRING:
				fprintf(stderr, "%s:%zu:%zu: error: missing terminating \" character\n", file_name, tok.line, tok.column);
				free(source);
				free(row_source);
				break;
			case INVALID:
				fprintf(stderr, "%s:%zu:%zu: warning: Invalid token %.*s\n", file_name, tok.line, tok.column, (int)tok.length, source + tok.start);

				fwrite(source + tok.start, 1, tok.length, fd);
				break;
			case COMMENT:
				fputc(' ', fd);
				continue;
			case NEWLINE:
				at_line_start = 1;
				fwrite(source + tok.start, 1, tok.length, fd);
				continue;
			case SPACE:
				// Whitespace at the beginning of a line
				// do not end the beginning-of-line state.
				fwrite(source + tok.start, 1, tok.length, fd);
				continue;
			case IDENTIFIER:
				res = expand_macros(tok, source, &ctx->macros, fd);
				if(res){
					free(source);
					free(row_source);
					return res;
				}
				break;
			default:
				at_line_start = 0;
				fwrite(source + tok.start, 1, tok.length, fd);
				break;
		}
	}
	free(source);
	source = NULL;
	free(row_source);
	row_source = NULL;
	return 0;
}

int handle_directive(const char *file_name, lexer *lex, FILE *fd, preprocessor_context *ctx, unsigned int depth){
	token tok;
	putc('\n',fd);
	do{
		tok = lexer_next(lex);
	}while(tok.type == SPACE || tok.type == COMMENT);

	//fprintf(stdout, "%.*s : %s \n", (int)tok.length, lex->source + tok.start, token_type_print(tok.type));
	if(tok.type != IDENTIFIER){
		fprintf(stderr, "%s:%zu:%zu: \033[1;31merror\033[0m: invalid preprocessing directive\n", file_name, tok.line, tok.column);
		return 1;
	}
	if(tok.length == 7 && strncmp(lex->source + tok.start, "include", 7) == 0){
		return handle_include(file_name, lex, fd, ctx, depth);
	}
	if(tok.length == 6 && strncmp(lex->source + tok.start, "define", 6) == 0){
		return handle_define(file_name, lex, &ctx->macros);
	}
	if (tok.length == 5 && strncmp(lex->source + tok.start, "undef", 5) == 0){
		return handle_undef(file_name, lex, &ctx->macros);
	}
	if (tok.length == 6 && strncmp(lex->source + tok.start, "ifndef", 6) == 0){
		return handle_ifndef(file_name, lex, ctx);
		//fprintf(stderr, "ifndef directive not implemented yet\n");
		//return 1;
	}
	if (tok.length == 5 && strncmp(lex->source + tok.start, "ifdef", 5) == 0){
		return handle_ifdef(file_name, lex, ctx);
		//fprintf(stderr, "ifdef directive not implemented yet\n");
		//return 1;
	}
	if (tok.length == 5 && strncmp(lex->source + tok.start, "endif", 5) == 0){
		return handle_endif(file_name, lex, tok, ctx);
		//fprintf(stderr, "endif directive not implemented yet\n");
		//return 1;
	}
	if (tok.length == 4 && strncmp(lex->source + tok.start, "else", 4) == 0){
		return handle_else(file_name, lex, tok, ctx);
		//fprintf(stderr, "else directive not implemented yet\n");
		//return 1;
	}
	fprintf(stderr, "%s:%zu:%zu: error: invalid preprocessing directive1\n", file_name, tok.line, tok.column);
	return 1;
}

int handle_include(const char *file_name, lexer *lex, FILE *fd, preprocessor_context *ctx, unsigned int depth){
	token tok;
	char *header_file_name = NULL;
	char *file_path = NULL;
	FILE *fs;
	int res;

	//Get the header filename
	do{
		tok = lexer_next(lex);
	}while(tok.type == SPACE || tok.type == COMMENT);

	if(tok.type == STRING){
		if(tok.length <= 2){
			fprintf(stderr, "%s:%zu:%zu: error: empty header file name\n", file_name, tok.line, tok.column);
			return 1;
		}
		header_file_name = malloc(tok.length - 1);
		if(!header_file_name){
			perror("malloc");
			return 1;
		}
		memcpy(header_file_name, lex->source + tok.start + 1, tok.length - 2);
		header_file_name[tok.length - 2] = '\0';

		fs = find_valid_path(file_name, header_file_name, &file_path);
	}
	else if(tok.type == OPERATOR && tok.length == 1 && lex->source[tok.start] == '<'){
		header_file_name = tok_to_str(lex);
		if(!header_file_name)
			return 1;

		fs = find_valid_path(NULL, header_file_name, &file_path);
	}
	else{
		fprintf(stderr, "%s:%zu:%zu: error: expected header filename\n",
				file_name, tok.line, tok.column);
		return 1;
	}

	if(!fs){
		free(header_file_name);
		return 1;
	}

	res = preprocessor(file_path, fs, fd, ctx, depth + 1);

	fclose(fs);
	free(header_file_name);
	free(file_path);

	return res;
}

