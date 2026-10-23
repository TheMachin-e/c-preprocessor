OBJ = src/lexer.o src/main.o src/preprocessor.o
CFLAGS = -Iinclude

mypreprocessor: $(OBJ) 
	cc $^ -o $@
%.o: %.c 
	cc $(CFLAGS) -c $< -o $@

clean:
	@rm -f $(OBJ) mypreprocessor 

