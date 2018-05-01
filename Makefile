all:
	flex Lex/lexical.l
	mv lex.yy.c Lex
	gcc Lex/main.c Lex/lex.yy.c -ll -o Lex/scanner
test:
	for file in ./Test
	do
		echo $file
		Lex/scanner file
	
