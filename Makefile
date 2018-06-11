all:
	bison -d Syntax/syntax.y
	mv syntax.tab.c	syntax.tab.h Syntax
	flex Lex/lexical.l
	mv lex.yy.c Lex
	gcc Lex/main.c Lex/tree.c Syntax/syntax.tab.c Semantics/symbol_table.c Semantics/semantic.c InterCode/intercode.c InterCode/symbol_table_intercodes.c -ll -ly -o parser

debug:
	bison -d -t Syntax/syntax.y
	mv syntax.tab.c	syntax.tab.h Syntax
	flex Lex/lexical.l
	mv lex.yy.c Lex
	gcc Lex/main.c Lex/tree.c Syntax/syntax.tab.c Semantics/symbol_table.c Semantics/semantic.c InterCode/intercode.c InterCode/symbol_table_intercodes.c -ll -ly -o parser

test1_1:
	./parser Test/SyntaTest/test1.cmm	

test1_2:
	./parser Test/SyntaTest/test2.cmm

test1_3:
	./parser Test/SyntaTest/test3.cmm

test1_4:
	./parser Test/SyntaTest/test4.cmm

test1_5:
	./parser Test/SyntaTest/test5.cmm

test1_6:
	./parser Test/SyntaTest/test6.cmm

test1_7:
	./parser Test/SyntaTest/test7.cmm

test1_8:
	./parser Test/SyntaTest/test8.cmm

test1_9:
	./parser Test/SyntaTest/test9.cmm	

test1_10:
	./parser Test/SyntaTest/test10.cmm

test2_1:
	./parser Test/SemanTest/test1.cmm

test2_2:
	./parser Test/SemanTest/test2.cmm

test2_3:
	./parser Test/SemanTest/test3.cmm

test2_4:
	./parser Test/SemanTest/test4.cmm

test2_5:
	./parser Test/SemanTest/test5.cmm

test2_6:
	./parser Test/SemanTest/test6.cmm

test2_7:
	./parser Test/SemanTest/test7.cmm

test2_8:
	./parser Test/SemanTest/test8.cmm

test2_9:
	./parser Test/SemanTest/test9.cmm

test2_10:
	./parser Test/SemanTest/test10.cmm

test2_11:
	./parser Test/SemanTest/test11.cmm

test2_12:
	./parser Test/SemanTest/test12.cmm

test2_13:
	./parser Test/SemanTest/test13.cmm

test2_14:
	./parser Test/SemanTest/test14.cmm

test2_15:
	./parser Test/SemanTest/test15.cmm

test2_16:
	./parser Test/SemanTest/test16.cmm

test2_17:
	./parser Test/SemanTest/test17.cmm

test2_18:
	./parser Test/SemanTest/test18.cmm

test2_19:
	./parser Test/SemanTest/test19.cmm

test2_20:
	./parser Test/SemanTest/test20.cmm

test2_21:
	./parser Test/SemanTest/test21.cmm

test2_22:
	./parser Test/SemanTest/test22.cmm

test2_23:
	./parser Test/SemanTest/test23.cmm

test3_1:
	./parser Test/InterCodeTest/test1 test1.ir

test3_2:
	./parser Test/InterCodeTest/test2 test2.ir

test3_3:
	./parser Test/InterCodeTest/test3 test3.ir
