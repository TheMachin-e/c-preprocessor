#include<ctype.h>
#include"lexer.h"
token lexer_next(lexer *ptr){
	token tok = {0};
	tok.type = INVALID;	
	tok.start = ptr->position;
	tok.line = ptr->line;
	tok.column = ptr->column;
	
	if (ptr->source[ptr->position] == ' ' || ptr->source[ptr->position] == '\v' || ptr->source[ptr->position] == '\f' || ptr->source[ptr->position] == '\r' || ptr->source[ptr->position] == '\t' ){
		tok.type = SPACE;
		
		while (ptr->source[ptr->position] == ' ' || ptr->source[ptr->position] == '\v' || ptr->source[ptr->position] == '\f' || ptr->source[ptr->position] == '\r' || ptr->source[ptr->position] == '\t' ){
		ptr->position++; 
		ptr->column++;
		}
		tok.length = ptr->position - tok.start;
		return tok;
	}

	if(isalpha(ptr->source[ptr->position]) || ptr->source[ptr->position] == '_'){
		tok.type = IDENTIFIER;
		while(isalnum(ptr->source[ptr->position]) || ptr->source[ptr->position] == '_'){
			ptr->position++;
			ptr->column++;
		}
	}
	else if(isdigit(ptr->source[ptr->position])){
		tok.type = NUMBER;
		if(ptr->source[ptr->position] == '0'){
			//ptr->position++;
			//ptr->column++;
			if((ptr->source[ptr->position + 1] | 32) == 'x'){
				ptr->position+=2;
				ptr->column+=2;
				while(isdigit(ptr->source[ptr->position]) || (ptr->source[ptr->position] | 32) == 'a' || (ptr->source[ptr->position] | 32) == 'b' || (ptr->source[ptr->position] | 32) == 'c' || (ptr->source[ptr->position] | 32) == 'd' || (ptr->source[ptr->position] | 32) == 'e' || (ptr->source[ptr->position] | 32) == 'f' ){
					ptr->position++;
					ptr->column++;
				}
				if(ptr->source[ptr->position] == '.'){
					ptr->position++;
					ptr->column++;
					while(isdigit(ptr->source[ptr->position]) || (ptr->source[ptr->position] | 32) == 'a' || (ptr->source[ptr->position] | 32) == 'b' || (ptr->source[ptr->position] | 32) == 'c' || (ptr->source[ptr->position] | 32) == 'd' || (ptr->source[ptr->position] | 32) == 'e' || (ptr->source[ptr->position] | 32) == 'f' ){
						ptr->position++;
						ptr->column++;
					}	
				}
				if((ptr->source[ptr->position] | 32) == 'p'){
					ptr->position++;
					ptr->column++;
					if(ptr->source[ptr->position] == '+' || ptr->source[ptr->position] == '-'){
						ptr->position++;
						ptr->column++;
					}
					while(isdigit(ptr->source[ptr->position])){
							ptr->position++;
							ptr->column++;
					}	
				}
				if((ptr->source[ptr->position] | 32) == 'f' || (ptr->source[ptr->position] | 32) == 'l'){
					ptr->position++;
					ptr->column++;
				}
			}
		}
		if(isdigit(ptr->source[ptr->position])){	
			ptr->position++;
			ptr->column++;
			while(isdigit(ptr->source[ptr->position])){
				ptr->position++;
				ptr->column++;
			}
			if(ptr->source[ptr->position] == '.'){
				ptr->position++;
				ptr->column++;
				while(isdigit(ptr->source[ptr->position])){
					ptr->position++;
					ptr->column++;
				}
				if((ptr->source[ptr->position] | 32) == 'e'){
					ptr->position++;
					ptr->column++;
					if(ptr->source[ptr->position] == '+' || ptr->source[ptr->position] == '-'){
						ptr->position++;
						ptr->column++;
					}
					while(isdigit(ptr->source[ptr->position])){
						ptr->position++;
						ptr->column++;
					}

				}
				if((ptr->source[ptr->position] | 32) == 'f' || (ptr->source[ptr->position] | 32) == 'l'){
					ptr->position++;
					ptr->column++;
				}
			}
			else if((ptr->source[ptr->position] | 32) == 'u'){
				ptr->position++;
				ptr->column++;
				if((ptr->source[ptr->position] | 32) == 'l'){
					ptr->position++;
					ptr->column++;
					if((ptr->source[ptr->position] | 32) == 'l'){
						ptr->position++;
						ptr->column++;
					}
				}
			} 
			else if((ptr->source[ptr->position] | 32) == 'l' ){
				ptr->position++;
				ptr->column++;
			}
			else if((ptr->source[ptr->position] | 32) == 'e'){
				ptr->position++;
				ptr->column++;
				if(ptr->source[ptr->position] == '+' || ptr->source[ptr->position] == '-'){
					ptr->position++;
					ptr->column++;
				}
				while(isdigit(ptr->source[ptr->position])){
					ptr->position++;
					ptr->column++;
				}
				if((ptr->source[ptr->position] | 32) == 'f' || (ptr->source[ptr->position] | 32) == 'l'){
					ptr->position++;
					ptr->column++;
				}
			}
		}
	}
	else if(ptr->source[ptr->position] == '.'){
		ptr->position++;
		ptr->column++;
		if(ptr->source[ptr->position] == '.'){
			ptr->position++;
			ptr->column++;
			if(ptr->source[ptr->position] == '.'){
				ptr->position++;
				ptr->column++;
				tok.type = OPERATOR;	
			}
		}
		else if(isdigit(ptr->source[ptr->position])){
			tok.type = NUMBER;
			ptr->position++;
			ptr->column++;
			while(isdigit(ptr->source[ptr->position])){
				ptr->position++;
				ptr->column++;
			}
			if((ptr->source[ptr->position] | 32) == 'e'){
				ptr->position++;
				ptr->column++;
				if(ptr->source[ptr->position] == '+' || ptr->source[ptr->position] == '-'){
					ptr->position++;
					ptr->column++;
				}
				while(isdigit(ptr->source[ptr->position])){
					ptr->position++;
					ptr->column++;
				}
			}
			if((ptr->source[ptr->position] | 32) == 'f' || (ptr->source[ptr->position] | 32) == 'l'){
				ptr->position++;
				ptr->column++;
			}
		}
		else
			tok.type = OPERATOR; 
	} 


	else if(ptr->source[ptr->position] == '#'){
		tok.type = HASH;
		ptr->position++;
		ptr->column++;
		if(ptr->source[ptr->position] == '#'){
			tok.type = HASH_HASH;
			ptr->position++;
			ptr->column++;
		}
	}
	else if(ptr->source[ptr->position] == '"'){
		tok.type = STRING;
		do{
			ptr->position++;
			ptr->column++;
			if(ptr->source[ptr->position] == '"'){
				ptr->position++;
				ptr->column++;
				break;	
			}
			else if(ptr->source[ptr->position] == '\\'){
				ptr->position++;
				ptr->column++;
			}
		}while(ptr->source[ptr->position]);
		if(ptr->source[ptr->position] == '\0' &&  ptr->source[ptr->position - 1] != '"')
			tok.type = UNTERM_STRING;
	}


	else if(ptr->source[ptr->position] == '\''){
		tok.type = CHAR;
		do{
			ptr->position++;
			ptr->column++;
			if(ptr->source[ptr->position] == '\''){
				ptr->position++;
				ptr->column++;
				break;
			}
			if(ptr->source[ptr->position] == '\\'){
				ptr->position++;
				ptr->column++;
			}
		}while(ptr->source[ptr->position]);
		if(ptr->source[ptr->position] == '\0' &&  ptr->source[ptr->position - 1] != '\'')
			tok.type = UNTERM_CHAR;
		
	} 


	else if(ptr->source[ptr->position] == '{' || ptr->source[ptr->position] == '}' || ptr->source[ptr->position] == '(' || ptr->source[ptr->position] == ')' || ptr->source[ptr->position] == '[' || ptr->source[ptr->position] == ']' || ptr->source[ptr->position] == ';' || ptr->source[ptr->position] == ','){
		tok.type = DELIMITER;
		ptr->position++;
		ptr->column++;
	}


	else if(ptr->source[ptr->position] == '~'){
		tok.type = OPERATOR;
		ptr->position++;
		ptr->column++;
	}


	else if(ptr->source[ptr->position] == '+' ){
		tok.type = OPERATOR;
		ptr->position++;
		ptr->column++;
		if(ptr->source[ptr->position] == '=' || ptr->source[ptr->position] == '+'){
			ptr->position++;
			ptr->column++;
		}
	}	


	else if(ptr->source[ptr->position] == '&' ){
		tok.type = OPERATOR;
		ptr->position++;
		ptr->column++;
		if(ptr->source[ptr->position] == '=' || ptr->source[ptr->position] == '&'){
			ptr->position++;
			ptr->column++;
		}
	}	
	else if(ptr->source[ptr->position] == '|' ){
		tok.type = OPERATOR;
		ptr->position++;
		ptr->column++;
		if(ptr->source[ptr->position] == '=' || ptr->source[ptr->position] == '|'){
			ptr->position++;
			ptr->column++;
		}
	}	

	else if(ptr->source[ptr->position] == '-' ){
		tok.type = OPERATOR;
		ptr->position++;
		ptr->column++;
		if(ptr->source[ptr->position] == '=' || ptr->source[ptr->position] == '-' || ptr->source[ptr->position] == '>'){
			ptr->position++;
			ptr->column++;
		}
	}
	else if(ptr->source[ptr->position] == '=' ){
		tok.type = OPERATOR;
		ptr->position++;
		ptr->column++;
		if(ptr->source[ptr->position] == '='){
			ptr->position++;
			ptr->column++;
		}
	}	
	else if(ptr->source[ptr->position] == '/' ){
		tok.type = OPERATOR;
		ptr->position++;
		ptr->column++;
		if(ptr->source[ptr->position] == '='){
			ptr->position++;
			ptr->column++;
		}
		else if(ptr->source[ptr->position] == '/'){ 
			tok.type = COMMENT;
			do{
			ptr->position++;
			ptr->column++;
			}while(ptr->source[ptr->position] && ptr->source[ptr->position] != '\n'); 
		}
		else if(ptr->source[ptr->position] == '*'){
			tok.type = COMMENT;
			do{
			if(ptr->source[ptr->position] == '\n'){
				ptr->position++;
				ptr->line++;
				ptr->column = 1;
			}
			else{
				ptr->position++;
				ptr->column++;
			}
			}while(ptr->source[ptr->position] && (ptr->source[ptr->position] != '*' || ptr->source[ptr->position + 1] != '/' )); 
			if(!ptr->source[ptr->position])
				tok.type = UNTERM_COMMENT;	
			else{
				ptr->position += 2;
				ptr->column += 2;	
			}
		}
		
	}
	else if(ptr->source[ptr->position] == '*' ){
		tok.type = OPERATOR;
		ptr->position++;
		ptr->column++;
		if(ptr->source[ptr->position] == '='){
			ptr->position++;
			ptr->column++;
		}
	}

	else if(ptr->source[ptr->position] == '%' ){
		tok.type = OPERATOR;
		ptr->position++;
		ptr->column++;
		if(ptr->source[ptr->position] == '='){
			ptr->position++;
			ptr->column++;
		}
	}
	else if(ptr->source[ptr->position] == '<' ){
		tok.type = OPERATOR;
		ptr->position++;
		ptr->column++;
		if(ptr->source[ptr->position] == '='){
			ptr->position++;
			ptr->column++;
		}
		else if(ptr->source[ptr->position] == '<'){
			ptr->position++;
			ptr->column++;
			if(ptr->source[ptr->position] == '='){
				ptr->position++;
				ptr->column++;
			}
		}
	}
	else if(ptr->source[ptr->position] == '>' ){
		tok.type = OPERATOR;
		ptr->position++;
		ptr->column++;
		if(ptr->source[ptr->position] == '='){
			ptr->position++;
			ptr->column++;
		}
		else if(ptr->source[ptr->position] == '>'){
			ptr->position++;
			ptr->column++;
			if(ptr->source[ptr->position] == '='){
				ptr->position++;
				ptr->column++;
			}
		}
	}

	else if(ptr->source[ptr->position] == '!' ){
		tok.type = OPERATOR;
		ptr->position++;
		ptr->column++;
		if(ptr->source[ptr->position] == '='){
			ptr->position++;
			ptr->column++;
		}
	}
	else if(ptr->source[ptr->position] == '^' ){
		tok.type = OPERATOR;
		ptr->position++;
		ptr->column++;
		if(ptr->source[ptr->position] == '='){
			ptr->position++;
			ptr->column++;
		}
	}
	else if(ptr->source[ptr->position] == '?' ){
		tok.type = OPERATOR;
		ptr->position++;
		ptr->column++;
	}
	else if(ptr->source[ptr->position] == ':' ){
		tok.type = OPERATOR;
		ptr->position++;
		ptr->column++;
	}	
	else if(ptr->source[ptr->position] == '\n'){
		tok.type = NEWLINE;
		ptr->line++;
		ptr->column = 1;
		ptr->position++;
	}
	else if(ptr->source[ptr->position] == '\0'){
		tok.type = TOKEN_EOF;
		tok.length = 0;
		return tok;
	}
	else{
		ptr->position++;
		ptr->column++;
	}
	tok.length = ptr->position - tok.start;
	return tok;

}


char *token_type_print(token_type type){
	switch(type){
		case INVALID: return "INVALID";
		case UNTERM_COMMENT: return "UNTERMINATED COMMENT";
		case UNTERM_CHAR: return "UNTERMINATED CHAR";
		case UNTERM_STRING: return "UNTERMINATED STRING";
		case IDENTIFIER: return "IDENTIFIER";
		case NUMBER: return "NUMBER";
		case STRING: return "STRING";
		case CHAR: return "CHAR";
		case COMMENT: return "COMMENT";
		case OPERATOR: return "OPERATOR";
		case DELIMITER: return "DELIMITER";
		case HASH: return "HASH";
		case HASH_HASH: return "DOUBLE HASH";
		case NEWLINE: return "NEWLINE";
		case SPACE: return "SPACE";
		case TOKEN_EOF: return "TOKEN_EOF"; 
		default: printf("unrecognized token\n");
	}
}

