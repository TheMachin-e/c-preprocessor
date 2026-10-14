#include<stdlib.h>
#include<string.h>
#include"lexer.h"
char *splice_remover(char *source, int len){
	char *new_s = malloc(sizeof(char) * len);
	int i = 0, j = 0;
	while(source[i]){
		if( source[i] != '\\' || source[i + 1] != '\n')
			new_s[j++] = source[i];
		else
			i++;
		i++;
	}
	new_s[j] = '\0';
	return new_s;
}
