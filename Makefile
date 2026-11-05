OBJ = src/lexer.o src/main.o src/preprocessor.o src/utilities.o src/macro_replacement.o src/conditional_compiling.o
CFLAGS = -Iinclude

mypreprocessor: $(OBJ) 
	cc $^ -o $@
%.o: %.c 
	cc $(CFLAGS) -c $< -o $@

clean:
	@rm -f $(OBJ) mypreprocessor 

