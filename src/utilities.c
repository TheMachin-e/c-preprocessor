#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include"lexer.h"

const char *include_dirs[] = {
	"/usr/lib/gcc/x86_64-linux-gnu/15/include/",
	"/usr/local/include/",
	"/usr/include/x86_64-linux-gnu/",
	"/usr/include/"
};

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

char *tok_to_str(lexer *lex){
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

FILE *find_valid_path(const char *source_file, const char *file_name,  char **file_path){
	FILE *fs = NULL;
	*file_path = NULL;
	if(source_file){ // source_file is passed null for angular bracket header inclusion
		*file_path = to_path(source_file, file_name);
		fprintf(stdout, "Including: %s ", *file_path);
		if(!*file_path){
			return NULL;
		}
		if(fs = fopen(*file_path, "r")){
			fprintf(stdout, "success\n");
			return fs;
		}
		fprintf(stdout, "failed\n");
		free(*file_path);
		*file_path = NULL;
	}
	size_t ele = sizeof(include_dirs) / sizeof(include_dirs[0]);
	for(size_t i = 0; i < ele; i++){
		*file_path = to_path(include_dirs[i], file_name);
		fprintf(stdout, "Including: %s ", *file_path);
		if(!file_path){
			return NULL;
		}
		if(fs = fopen(*file_path, "r")){
			fprintf(stdout, "success\n");
			return fs;
		}
		fprintf(stdout, "failed\n");
		free(*file_path); 
		*file_path = NULL;
	}
	perror(file_name);
	return NULL;
}

void free_replacement_tokens(macro *node){
	for(size_t i = 0; i < node->replacement_count; i++)
		free(node->replacement[i].text);

	free(node->replacement);
	node->replacement = NULL;
	node->replacement_count = 0;
}

size_t djb2(const char *str){
	size_t hash = 5381;
	while(*str)
		hash = hash * 33  + (unsigned char)*str++;
	return hash;
}

size_t macro_hash(const char *name, const size_t bucket_count){
	return djb2(name) % bucket_count;
}

void insert_macro(macro_table *table, macro *new_macro){
	size_t hash = macro_hash(new_macro->name, table->bucket_count);
	macro *node = table->buckets[hash];
	if(!node){
		table->buckets[hash] = new_macro;	
		table->macro_count += 1;
		return;
	}
	while(node){
		if(strcmp(node->name, new_macro->name) == 0){
			free_replacement_token(node);
			node->replacement = new_macro->replacement;
			node->replacement_count = new_macro->replacement_count;
			new_macro->replacemnet = NULL;
			free(new_macro->name);
			free(new_macro);
			return;
		}
		if(!node->next){
			node->next = new_macro;
			table->macro_count += 1;
			return;
		}
		node = node->next;
	}
}

macro *macro_lookup(const macro_table *table, const char *name){
	size_t hash = macro_hash(name, table->bucket_count);
	macro *node = table->buckets[hash];
	while(node){
		if(strcmp(node->name, name) == 0)
			return node;
		node = node->next;  
	}
	return NULL;
}

void macro_delete(macro_table * table, const char *name){
	size_t hash = macro_hash(name, table->bucket_count);
	macro *prev = NULL;
	macro *node = table->buckets[hash];
	while(node){
		if(strcmp(node->name, name) == 0){
			if(prev)
				prev->next = node->next;
			else
				table->buckets[hash] = node->next;	
			
			free_replacement_tokens(node);
			free(node->name);
			free(node);
			table->macro_count--;
			return;	
		}
		prev = node;
		node = node->next;
	}
}

void macro_table_destroy(macro_table *table){
	macro * node;
	for(size_t i = 0; i < table->bucket_count; i++){
		node = table->buckets[i];
		while(node){
			table->buckets[i] = node->next;
			freer_replacement_tokens(node);
			free(node->name);
			free(node);
			node = table->buckets[i];
		}
	}
	free(table->buckets);
	table->buckets = NULL;
	table->bucket_count = 0;
	table->macro_count = 0;
}
