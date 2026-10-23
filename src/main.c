#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include"lexer.h"
int main(int argc, char **argv){
	if(argc != 2){
		printf("Usage: ./mypreprocessor file_name.c");
		return 1;
	}
	FILE *fp = fopen(argv[1], "r");
	if(!fp){
		perror("fopen");
		return 1;
	}
	size_t len = strlen(argv[1]);
	char *des = malloc(sizeof(char) * (len + 1));
	if(!des){
		perror("malloc");
		fclose(fp);
		return 1;
	}
	strcpy(des, argv[1]);
	des[len - 1] = 'i';
	FILE *fd = fopen(des, "w");
	if(!fd){
		perror("fopen");
		fclose(fp);
		free(des);
		des = NULL;
		return 1;
	}
	free(des);
	des = NULL;
	fseek(fp, 0, SEEK_END);
	size_t file_len = ftell(fp);
	size_t source_len = file_len + 1;
	rewind(fp);
	char *row_source = malloc(sizeof(char) * source_len);
	if(!row_source){
		perror("malloc");
		fclose(fp);
		fclose(fd);
		return 1;
	}
	fread(row_source, 1, file_len, fp);
	row_source[file_len] = '\0';
	char *source = splice_remover(row_source, &source_len);
	if(!source){
		perror("malloc");
		fclose(fp);
		fclose(fd);
		free(row_source);
		row_source = NULL;
		return 1;	
	}
	printf("%s\n", source);
	lexer lex = {source, 0, source_len, 1, 1};
	token tok = {0};
	while((tok = lexer_next(&lex)).type != TOKEN_EOF){
		printf("%-30s %-20.*s line = %zu column = %zu\n", token_type_print(tok.type), (int)tok.length, source + tok.start, tok.line, tok.column);
		if(tok.type == COMMENT){
			remove_comments(source + tok.start, tok.length);
		}
	}
	fwrite(source, 1, source_len, fd);
	free(source);
	source = NULL;
	free(row_source);
	row_source = NULL;
	fclose(fp);
	fclose(fd);
	return 0;
}
