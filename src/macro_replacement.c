#include<stdlib.h>
#include<lexer.h>
#include<string.h>
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



