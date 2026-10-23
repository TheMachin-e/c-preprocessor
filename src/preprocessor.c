#include<stdlib.h>
#include<string.h>
#include"lexer.h"
char *splice_remover(char *source, size_t *len){
	char *new_s = malloc(sizeof(char) * *len);
	if(!new_s){
		return NULL;
	}
	size_t i = 0, j = 0;
	while(source[i]){
		if( source[i] != '\\' || source[i + 1] != '\n')
			new_s[j++] = source[i];
		else
			i++;
		i++;
	}
	new_s[j] = '\0';
	*len = j;
	return new_s;
}

void remove_comments(char *source, size_t length){
	for(size_t i = 0; i < length; i++){
			source[i] = ' ';
	}
}



