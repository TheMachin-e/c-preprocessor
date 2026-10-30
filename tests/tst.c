#include/*
*/<stdio.h>
#define PI 3.14
#define SQUARE(x)\
  ((x) * (x))

int      main(void)
{
	// Calculate a value
	float x \
= SQUARE(5) + PI;
	char           */*hi*/message = "hello // world";	

    return                       0;
}
/*       
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
		printf("No such file or directory\n");
		return 1;
	}
	size_t len = strlen(argv[1]);
	char *des = malloc(sizeof(char) * (len + 1));
	strcpy(des, argv[1]);
	des[len - 1] = 'i';
	FILE *fd = fopen(des, "w");
	free(des);
	fseek(fp, 0, SEEK_END);
	size_t file_len = ftell(fp);
	size_t source_len = file_len + 1;
	rewind(fp);
	char *row_source = malloc(sizeof(char) * source_len);
	fread(row_source, 1, file_len, fp);
	row_source[file_len] = '\0';
	char *source = splice_remover(row_source, &source_len);
	printf("%s\n", source);
	lexer lex = {source, 0, source_len, 1, 1};
	token tok = {0};
	while((tok = lexer_next(&lex)).type != TOKEN_EOF){
		printf("%-30s %-20.*s line = %zu column = %zu\n", token_type_print(tok.type), (int)tok.length, source + tok.start, tok.line, tok.column);
	}
	free(source);
	free(row_source);
	fclose(fp);
	fclose(fd);
	return 0;

	hi +++ ++++ === ==== &&& &&&&. .. ... .....
	$$$
	>>= 
}
 */
