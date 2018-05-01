INT10 ([1-9][0-9]*) | 0
INT8 0[0-7]*
INT16 ("0x"|"0X")[0-9A-Fa-f]+
INT {INT10} | {INT8} | {INT16}

FLOAT1 [0-9]+.[0-9]+
FLOAT2 (([0-9]+"."[0-9]*)|([0-9]*"."[0-9]+))[Ee][+-]?[0-9]+

ID [a-zA-Z_][0-9a-zA-Z_]*

SEMI ";"
COMMA ","
ASSIGNOP "="
RELOP ">"|"<"|">="|"<="|"=="|"!="
PLUS "+"
MINUS "-"
STAR "*"
DIV "/"
AND "&&"
OR "||"
DOT "."
NOT "!"
TYPE "int" | "float"
LP "("
RP ")"
LB "["
RB "]"
LC "{"
RC "}"
STRUCT "struct"
RETURN "return"
IF "if"
ELSE "else"
WHILE "while"
%%
{letter}+ { words++; chars+= yyleng; }
\n { chars++; lines++;}
. {chars++;}
%%
int main(int argc, char** argv) {
	if(argc > 1) {
		if(!(yyin = fopen(argv[1], "r"))) {
			perror(argv[1]);
			return 1;
		}
	}
	yylex();
	printf("%8d%8d%8d\n", lines, words, chars);
	return 0;
}
