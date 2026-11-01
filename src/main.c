#include<string.h>
#include<stdlib.h>
#include"lexer.h"


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
	
	FILE *fp = fopen(argv[1], "r");
	if(!fp){
		perror(argv[1]);
		fclose(fd);
		remove(des);
		free(des);
		des = NULL;
		return 1;
	}

	res = preprocessor(argv[1], fp, fd, 0);

	fclose(fd);
	fclose(fp);

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
