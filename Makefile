all:
	flex Lex/lexical.l
	mv lex.yy.c Lex
	bison -d Syntax/syntax.y
	mv syntax.tab.c	syntax.tab.h Syntax
	gcc Lex/main.c -ll -ly -o parser

test1:
	./parser Test/test1.cmm			
