OBJ = src/lexer.o src/main.o src/splice_remover.o
CFLAGS = -Iinclude

mypreprocessor: $(OBJ) 
	cc $^ -o $@
%.o: %.c 
	cc $(CFLAGS) -c $< -o $@

clean:
	@rm -f $(OBJ) mypreprocessor 

