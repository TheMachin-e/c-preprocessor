#include<stdlib.h>
#include<stdio.h>
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

char * tok_to_str(lexer *lex){
	token tok;
	size_t len = 0;
	size_t mem_len = 30;
	size_t i = 0;
	char *file_name = malloc(sizeof(char) * mem_len),  *tmp;
	if(!file_name){
		perror("malloc");
		return NULL;
	}
	while((tok = lexer_next(lex)).type != TOKEN_EOF){
		if(tok.type == NEWLINE){
			free(file_name);
			file_name = NULL;
			//error report
			return NULL;
		}
		else if(tok.type == OPERATOR && tok.length == 1 && lex->source[tok.start] == '>')
			break;
		i = len;
		len += tok.length;
		if(len >= mem_len){
			while(len + 1 > mem_len)
				mem_len *= 2;
			tmp = realloc(file_name, mem_len);
			if(!tmp){
				perror("realloc");
				free(file_name);
				return NULL;
			}
			file_name = tmp;
			tmp = NULL;
		}
		memcpy(file_name + i, lex->source + tok.start, tok.length);
	}
	if(tok.type == TOKEN_EOF){
		free(file_name);
		file_name = NULL;
		//error report
		return NULL;
	}
	file_name[len] = '\0';
	return file_name;
}

char *to_path(const char *source_file, const char *file_name){
	size_t path_len;
	char *path;
		const char *slash = strrchr(source_file, '/');
		size_t len;
		if(slash){
			len = (slash - source_file) + 1;
			path_len = len  + strlen(file_name) + 1;
		}
		else 
			path_len = (strlen(file_name) + 3);
	
		path = malloc(path_len);
		if(!path){
			perror("malloc");
			return NULL;
		}
		if(slash){
			memcpy(path, source_file, len);
			path[len] = '\0';
		}
		else{
			memcpy(path, "./", 3);
		}
		strcat(path, file_name);
		return path;
}


