#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include"lexer.h"
#define MAX_INCLUDE_DEPTH 100
int preprocessor(const char *, FILE *, unsigned int);

int main(int argc, char **argv){
	int res;
	if(argc != 2){
		printf("Usage: ./mypreprocessor file_name.c");
		return 1;
	}
	size_t len = strlen(argv[1]);
	if(len < 3 || strcmp(argv[1] + (len - 2), ".c") != 0){ 	//check again
		fprintf(stderr, "Invalid file format\n");
		return 1;
	}
	char *des = malloc(sizeof(char) * (len + 1));
	if(!des){
		perror("malloc");
		return 1;
	}
	strcpy(des, argv[1]);
	des[len - 1] = 'i';
	FILE *fd = fopen(des, "w");
	if(!fd){
		perror("fopen");
		free(des);
		des = NULL;
		return 1;
	}
	res = preprocessor(argv[1], fd, 0);
	fclose(fd);
	if(res != 0){
		if(remove(des) == -1){
			perror("remove");
			return 1;
		}
	}
	free(des);
	des = NULL;
	return res;
}

int preprocessor(const char *file_name, FILE *fd, unsigned int depth){
	if(depth >= MAX_INCLUDE_DEPTH){
		fprintf(stderr, "#include nested depth exceeded 100\n");
		return 1;
	}
	FILE *fp = fopen(file_name, "r");
	if(!fp){
		perror(file_name);
		return 1;
	}
	if(fseek(fp, 0, SEEK_END) != 0){
		perror("fseek");
		fclose(fp);
		return 1;
	}
	long file_len = ftell(fp);
	if(file_len < 0){
		perror("ftell");
		fclose(fp);
		return 1;
	}
	size_t source_len = (size_t)file_len + 1;
	rewind(fp);
	char *row_source = malloc(sizeof(char) * source_len);
	if(!row_source){
		perror("malloc");
		fclose(fp);
		return 1;
	}
	if(fread(row_source, 1, (size_t)file_len, fp) != (size_t)file_len){
		fprintf(stderr, "Failed to read input file\n");
		free(row_source);
		fclose(fp);
		return 1;
	}
	row_source[file_len] = '\0';
	char *source = splice_remover(row_source, &source_len);
	if(!source){
		perror("malloc");
		fclose(fp);
		free(row_source);
		row_source = NULL;
		return 1;	
	}
	char *header_file_name;
	char *file_path;
	lexer lex = {source, 0, source_len, 1, 1};
	token tok = {0};
	int directive_flag = 0;
	int res;
	while((tok = lexer_next(&lex)).type != TOKEN_EOF){
		switch(tok.type){
			case UNTERM_COMMENT: 
				fprintf(stderr, "%d:%d: error: unterminated comment\n", tok.line, tok.column);
				return 1;
			case UNTERM_CHAR: 
				fprintf(stderr, "%d:%d: warning: missing terminating ' character\n", tok.line, tok.column);
				break;
			case UNTERM_STRING: 
				fprintf(stderr, "%d:%d: warning: missing terminating \" character\n", tok.line, tok.column);
				break;
			case INVALID:
				fprintf(stderr, "%d:%d: Invalid token", tok.line, tok.column);
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
						fclose(fp);
						free(source);
						source = NULL;
						free(row_source);
						row_source = NULL;
						return 1;	
					}	
					header_file_name = malloc(sizeof(char) * tok.length - 1);
					if(!header_file_name){
						perror("malloc");
						fclose(fp);
						free(source);
						source = NULL;
						free(row_source);
						row_source = NULL;
						return 1;	

					}
					strncpy(header_file_name, source + tok.start + 1, tok.length - 1);// last " included // beware of ""
					header_file_name[tok.length - 2] = '\0';	
					file_path = to_path()
					res = preprocessor(header_file_name, fd, depth + 1);
					free(header_file_name);
					header_file_name = NULL;
					if(res != 0){
						fclose(fp);
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
							fclose(fp);
							free(source);
							source = NULL;
							free(row_source);
							row_source = NULL;
							return 1;	
						}
						file_path = to_path(header_file_name);
						if(!file_path){
							fclose(fp);
							free(source);
							source = NULL;
							free(row_source);
							row_source = NULL;
							return 1;	
						}
						res = preprocessor(file_path, fd, depth + 1);
						free(header_file_name);
						header_file_name = NULL;
						free(file_path);
						file_path = NULL;
						if(res != 0){
							fclose(fp);
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
			default: 
				fwrite(source + tok.start, 1, tok.length, fd);
		}
	}
	free(source);
	source = NULL;
	free(row_source);
	row_source = NULL;
	fclose(fp);
	return 0;
}

