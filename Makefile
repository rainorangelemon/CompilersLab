all:
	bison -d Syntax/syntax.y
	mv syntax.tab.c	syntax.tab.h Syntax
	flex Lex/lexical.l
	mv lex.yy.c Lex
	gcc Lex/main.c Lex/tree.c Syntax/syntax.tab.c Semantics/symbol_table.c Semantics/semantic.c -ll -ly -o parser

debug:
	bison -d -t Syntax/syntax.y
	mv syntax.tab.c	syntax.tab.h Syntax
	flex Lex/lexical.l
	mv lex.yy.c Lex
	gcc Lex/main.c Lex/tree.c Syntax/syntax.tab.c Semantics/symbol_table.c Semantics/semantic.c -ll -ly -o parser

testSyntax1:
	./parser Test/SyntaTest/test1.cmm	

testSyntax2:
	./parser Test/SyntaTest/test2.cmm

testSyntax3:
	./parser Test/SyntaTest/test3.cmm

testSyntax4:
	./parser Test/SyntaTest/test4.cmm

testSyntax5:
	./parser Test/SyntaTest/test5.cmm

testSyntax6:
	./parser Test/SyntaTest/test6.cmm

testSyntax7:
	./parser Test/SyntaTest/test7.cmm

testSyntax8:
	./parser Test/SyntaTest/test8.cmm

testSyntax9:
	./parser Test/SyntaTest/test9.cmm	

testSyntax10:
	./parser Test/SyntaTest/test10.cmm

testSeman1:
	./parser Test/SemanTest/test1.cmm

testSeman2:
	./parser Test/SemanTest/test2.cmm

testSeman3:
	./parser Test/SemanTest/test3.cmm

testSeman4:
	./parser Test/SemanTest/test4.cmm

testSeman5:
	./parser Test/SemanTest/test5.cmm

testSeman6:
	./parser Test/SemanTest/test6.cmm

testSeman7:
	./parser Test/SemanTest/test7.cmm

testSeman8:
	./parser Test/SemanTest/test8.cmm

testSeman9:
	./parser Test/SemanTest/test9.cmm

testSeman10:
	./parser Test/SemanTest/test10.cmm

testSeman11:
	./parser Test/SemanTest/test11.cmm

testSeman12:
	./parser Test/SemanTest/test12.cmm

testSeman13:
	./parser Test/SemanTest/test13.cmm

testSeman14:
	./parser Test/SemanTest/test14.cmm

testSeman15:
	./parser Test/SemanTest/test15.cmm

testSeman16:
	./parser Test/SemanTest/test16.cmm

testSeman17:
	./parser Test/SemanTest/test17.cmm

testSeman18:
	./parser Test/SemanTest/test18.cmm

testSeman19:
	./parser Test/SemanTest/test19.cmm

testSeman20:
	./parser Test/SemanTest/test20.cmm

testSeman21:
	./parser Test/SemanTest/test21.cmm

testSeman22:
	./parser Test/SemanTest/test22.cmm

testSeman23:
	./parser Test/SemanTest/test23.cmm
