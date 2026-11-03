#include<stdlib.h>
#include<string.h>
#include"lexer.h"
#define MAX_INCLUDE_DEPTH 100
#define INITIAL_BUCKET_COUNT 16


int preprocessor_context_init(preprocessor_context *ctx){
	ctx->macros.bucket_count = INITIAL_BUCKET_COUNT;
	ctx->macros.macro_count = 0;
	ctx->macros.buckets = calloc(ctx->macors.bucket_count, sizeof(macro *));
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
	int directive_flag = 0;
	int newline_flag = 0;
	int res;
	FILE * fs;
	while((tok = lexer_next(&lex)).type != TOKEN_EOF){
		switch(tok.type){
			case UNTERM_COMMENT: 
				fprintf(stderr, "%s:%ld:%ld: error: unterminated comment\n", file_name, tok.line, tok.column);
				free(source);
				source = NULL;
				free(row_source);
				row_source = NULL;
				return 1;
			case UNTERM_CHAR: 
				fprintf(stderr, "%s:%ld:%ld: warning: missing terminating ' character\n", file_name, tok.line, tok.column);
				fwrite(source + tok.start, 1, tok.length, fd);
				break;
			case UNTERM_STRING: 
				fprintf(stderr, "%s:%ld:%ld: warning: missing terminating \" character\n", file_name, tok.line, tok.column);
				fwrite(source + tok.start, 1, tok.length, fd);
				break;
			case INVALID:
				fprintf(stderr, "%s:%ld:%ld: warning: Invalid token %.*s\n", file_name, tok.line, tok.column, (int)tok.length, source + tok.start);
				fwrite(source + tok.start, 1, tok.length, fd);
				break;
			case COMMENT:
				fputc(' ', fd);
				continue;

			case HASH:
				directive_flag = 1;
				break;
			case IDENTIFIER:
				if(directive_flag == 1)
					if(tok.length == 7 && strncmp(source + tok.start, "include", 7) == 0) //need to handle #define, #if etc..
						directive_flag = 2;
					else 
						directive_flag = 0;
				else
					fwrite(source + tok.start, 1, tok.length, fd);
				break;
			case STRING:
				if(directive_flag == 2){
					if(tok.length <= 2){
						free(source);
						source = NULL;
						free(row_source);
						row_source = NULL;
						return 1;	
					}	
					header_file_name = malloc(sizeof(char) * tok.length - 1);
					if(!header_file_name){
						perror("malloc");
						free(source);
						source = NULL;
						free(row_source);
						row_source = NULL;
						return 1;	

					}
					strncpy(header_file_name, source + tok.start + 1, tok.length - 1);// last " included // beware of ""
					header_file_name[tok.length - 2] = '\0';	
					//file_path = to_path(file_name, header_file_name);
					fs = find_valid_path(file_name, header_file_name, &file_path);
					if(!fs){
						free(source);
						source = NULL;
						free(row_source);
						row_source = NULL;
						free(header_file_name);
						header_file_name = NULL;
						return 1;	
					}

					res = preprocessor(file_path, fs, fd, ctx, depth + 1);

					fclose(fs);
					free(header_file_name);
					header_file_name = NULL;
					free(file_path);
					file_path = NULL;
					if(res != 0){
						free(source);
						source = NULL;
						free(row_source);
						row_source = NULL;
						return res;	
					}
					directive_flag = 0;
				}
				else
					fwrite(source + tok.start, 1, tok.length, fd);
				break;
			case OPERATOR:
				if(directive_flag == 2){
					if(strncmp(source + tok.start, "<", tok.length) == 0){
						header_file_name = tok_to_str(&lex);	
						if(!header_file_name){//better error handling needed
							perror("malloc");
							free(source);
							source = NULL;
							free(row_source);
							row_source = NULL;
							return 1;	
						}

						fs = find_valid_path(NULL, header_file_name, &file_path);
						if(!fs){
							free(source);
							source = NULL;
							free(row_source);
							row_source = NULL;
							free(header_file_name);
							header_file_name = NULL;
							return 1;	
						}


						res = preprocessor(file_path, fs, fd, ctx, depth + 1);

						fclose(fs);
						free(header_file_name);
						header_file_name = NULL;
						free(file_path);
						file_path = NULL;
						if(res != 0){
							free(source);
							source = NULL;
							free(row_source);
							row_source = NULL;
							return res;	
						}
						directive_flag = 0;
					}
				}
				else
					fwrite(source + tok.start, 1, tok.length, fd);
				break;
			case NEWLINE: 
				directive_flag = 0;
				newline_flag = 1;
			default: 
				fwrite(source + tok.start, 1, tok.length, fd);
		}
	}
	free(source);
	source = NULL;
	free(row_source);
	row_source = NULL;
	return 0;
}


