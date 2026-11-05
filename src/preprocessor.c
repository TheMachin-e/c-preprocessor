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
				fprintf(stderr, "%s:%zu:%zu: warning: missing terminating ' character\n", file_name, tok.line, tok.column);

				fwrite(source + tok.start, 1, tok.length, fd);
				break;
			case UNTERM_STRING:
				fprintf(stderr, "%s:%zu:%zu: warning: missing terminating \" character\n", file_name, tok.line, tok.column);

				fwrite(source + tok.start, 1, tok.length, fd);
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
		//fprintf(stderr, "undef directive not implemented yet\n");
		//return 1;
	}
	if (tok.length == 5 && strncmp(lex->source + tok.start, "undef", 5) == 0){
		return handle_undef(file_name, lex, &ctx->macros);
	}
	if (tok.length == 6 && strncmp(lex->source + tok.start, "ifndef", 6) == 0){
		//handle_ifndef()
		fprintf(stderr, "ifndef directive not implemented yet\n");
		return 1;
	}
	if (tok.length == 5 && strncmp(lex->source + tok.start, "ifdef", 5) == 0){
		//handle_ifdef()
		fprintf(stderr, "ifdef directive not implemented yet\n");
		return 1;
	}
	if (tok.length == 5 && strncmp(lex->source + tok.start, "endif", 5) == 0){
		//handle_endif()
		fprintf(stderr, "endif directive not implemented yet\n");
		return 1;
	}
	if (tok.length == 4 && strncmp(lex->source + tok.start, "else", 4) == 0){
		//handle_else()
		fprintf(stderr, "else directive not implemented yet\n");
		return 1;
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

int handle_define(const char *file_name, lexer *lex, macro_table *table){
	token tok;
	macro *new_macro = calloc(1, sizeof(macro));
	if(!new_macro){
		perror("calloc");
		return 1;
	}

	do{
		tok = lexer_next(lex);
	}while(tok.type == SPACE || tok.type == COMMENT);

	if(tok.type == NEWLINE || tok.type == TOKEN_EOF){
		fprintf(stderr, "%s:%zu:%zu: error: no macro name given with '#define' directive\n", file_name, tok.line, tok.column);
		free(new_macro);
		return 1;
	}	
	else if(tok.type != IDENTIFIER){
		fprintf(stderr, "%s:%zu:%zu: error: macro names must be identifier\n", file_name, tok.line, tok.column);
		free(new_macro);
		return 1;
	}

	new_macro->name = malloc(tok.length + 1);
	if(!new_macro->name){
		free(new_macro);
		perror("malloc");
		return 1;
	}	
	memcpy(new_macro->name, lex->source + tok.start, tok.length);
	new_macro->name[tok.length] = '\0';

	tok = lexer_next(lex);

	if(tok.type != SPACE && tok.type != NEWLINE)
		fprintf(stderr, "%s:%zu:%zu: warning: ISO C99 requires whitespace after the macro name\n", file_name, tok.line, tok.column);
	
	//function macro not implemented
	if(tok.type == DELIMITER && tok.length == 1 && lex->source[tok.start] == '('){
		fprintf(stderr, "%s:%zu:%zu: error: function-like macros not supported yet\n", file_name, tok.line, tok.column);
		free(new_macro->name);
		free(new_macro);
		return 1;
	}

	while(tok.type == SPACE || tok.type == COMMENT)
		tok = lexer_next(lex);

	if(tok.type == NEWLINE || tok.type == TOKEN_EOF){
		insert_macro(table, new_macro);
		return 0;
	}

	size_t mem_size = 4;
	new_macro->replacement = calloc(mem_size, sizeof(macro_token));
	if(!new_macro->replacement){
		free(new_macro->name);
		free(new_macro);
		perror("calloc");
		return 1;
	}
	macro_token *tmp;
	while(tok.type != NEWLINE && tok.type != TOKEN_EOF){
		if(tok.type == UNTERM_COMMENT || tok.type == UNTERM_STRING || tok.type == UNTERM_CHAR){
			fprintf(stderr, "%s:%zu:%zu: error: unterminated token in macro replacement\n", file_name, tok.line, tok.column);

			free_replacement_tokens(new_macro);
			free(new_macro->name);
			free(new_macro);
			return 1;
		}

		if(new_macro->replacement_count >= mem_size){
			mem_size *= 2;
			tmp = realloc(new_macro->replacement, mem_size * sizeof(macro_token));
			if(!tmp){
				perror("realloc");
				free_replacement_tokens(new_macro);
				free(new_macro->name);
				free(new_macro);
				return 1;
			}
			new_macro->replacement = tmp;
		}	

		size_t index = new_macro->replacement_count;
		if(tok.type == COMMENT){
			new_macro->replacement[index].type = SPACE;
			new_macro->replacement[index].text = malloc(sizeof(char) * 2);
			if(!new_macro->replacement[index].text){
				free_replacement_tokens(new_macro);
				free(new_macro->name);
				free(new_macro);
				perror("malloc");
				return 1;
			}
			strcpy(new_macro->replacement[index].text, " ");
		}
		else{	
			new_macro->replacement[index].type = tok.type;
			new_macro->replacement[index].text = malloc(tok.length + 1);
			if(!new_macro->replacement[index].text){
				free_replacement_tokens(new_macro);
				free(new_macro->name);
				free(new_macro->name);
				perror("malloc");
				return 1;
			}
			memcpy(new_macro->replacement[index].text, lex->source + tok.start, tok.length);
			new_macro->replacement[index].text[tok.length] = '\0';
		}
		new_macro->replacement_count++;
		tok = lexer_next(lex);
	}
	insert_macro(table, new_macro);
	return 0;
}

int handle_undef(const char *file_name, lexer *lex, macro_table *table){
	token tok;
	do{
		tok = lexer_next(lex);
	}while(tok.type == SPACE || tok.type == COMMENT);

	if(tok.type == NEWLINE || tok.type == TOKEN_EOF){
		fprintf(stderr, "%s:%zu:%zu: \033[1;31merror\033[0m: no macro name given in \033[1m'#undef'\033[0m directive\n", file_name, tok.line, tok.column);
		return 1;
	}
	else if(tok.type != IDENTIFIER){
		fprintf(stderr, "%s:%zu:%zu: error: macro name must identifier\n", file_name, tok.line, tok.column);
		return 1;
	}
	
	char *macro_name = malloc(tok.length + 1);
	if(!macro_name){
		perror("malloc");
		return 1;
	}
	memcpy(macro_name, lex->source + tok.start, tok.length);
	macro_name[tok.length] = '\0';
	macro_delete(table, macro_name);
	free(macro_name);
	char flag = 0;//to prevent the warning from printing more than onece

	while((tok = lexer_next(lex)).type != NEWLINE && tok.type != TOKEN_EOF){
		if(tok.type != SPACE && tok.type != COMMENT && !flag){
			fprintf(stderr, "%s:%zu:%zu: warning: extra tokens at the end of '#undef' directive\n", file_name, tok.line, tok.column);
			flag = 1;
		}
		if(tok.type == UNTERM_COMMENT || tok.type == UNTERM_STRING || tok.type == UNTERM_CHAR){
			fprintf(stderr, "%s:%zu:%zu: error: unterminated token at the end of '#undef' directive\n", file_name, tok.line, tok.column);
			return 1;	
		}
	}
	return 0;
}

int expand_macros(const token tok, const char *source, const macro_table *table, FILE *fd){
	char *identifier = malloc(tok.length + 1);
	if(!identifier){
		perror("malloc");
		return 1;
	}
	memcpy(identifier, source + tok.start, tok.length);
	identifier[tok.length] = '\0';
	macro *tmp_macro = macro_lookup(table, identifier);
	if(!tmp_macro){
		fputs(identifier, fd);
		free(identifier);
		return 0;
	}
	free(identifier);
	tmp_macro->lookup_flag = 1;
	expand_token(tmp_macro, table, fd);
	tmp_macro->lookup_flag = 0;
	return 0;
}

void  expand_token(macro *tmp_macro,  const macro_table *table, FILE *fd){
	if(!tmp_macro->replacement){
		return;
	}
	size_t count = tmp_macro->replacement_count;
	char *name;
	macro *rep_macro;
	for(size_t i = 0; i < count; i++){
		name = tmp_macro->replacement[i].text;
		if(tmp_macro->replacement[i].type == IDENTIFIER){
			rep_macro = macro_lookup(table, name);
			if(!rep_macro || rep_macro->lookup_flag){
				fputs(name, fd);
			}
			else{	
				rep_macro->lookup_flag = 1;
				expand_token(rep_macro, table, fd);
				rep_macro->lookup_flag = 0;
			}
		}
		else
			fputs(name, fd);
	}

}












