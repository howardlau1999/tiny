out: tiny

tiny: lex.yy.c y.tab.c
	gcc -o $@ $^

lex.yy.c: tiny.l y.tab.c
	flex $<

y.tab.c: tiny.y
	bison -vdty $<