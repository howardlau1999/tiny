out: tiny

tiny: parser.o token.o lexer.o
	gcc -o $@ $^

%.o: %.c token.h
	gcc -c -o $@ $<