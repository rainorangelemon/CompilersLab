all:
	bison -d Syntax/syntax.y
	mv syntax.tab.c	syntax.tab.h Syntax
	flex Lex/lexical.l
	mv lex.yy.c Lex
	gcc Lex/main.c Syntax/syntax.tab.c -ll -ly -o parser

debug:
	bison -d -t Syntax/syntax.y
	mv syntax.tab.c	syntax.tab.h Syntax
	flex Lex/lexical.l
	mv lex.yy.c Lex
	gcc Lex/main.c Syntax/syntax.tab.c -ll -ly -o parser
test1:
	./parser Test/test1.cmm	
test2:
	./parser Test/test2.cmm
test3:
	./parser Test/test3.cmm
test4:
	./parser Test/test4.cmm
test5:
	./parser Test/test5.cmm
test6:
	./parser Test/test6.cmm
test7:
	./parser Test/test7.cmm
test8:
	./parser Test/test8.cmm
test9:
	./parser Test/test9.cmm	
test10:
	./parser Test/test10.cmm
test11:
	./parser Test/test11.cmm	
test12:
	./parser Test/test12.cmm	
