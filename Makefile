compilador: parser.y scanner.l
	yacc -d parser.y --report all
	flex scanner.l
	gcc -c y.tab.c lex.yy.c
	gcc y.tab.o lex.yy.o cgen.c -o compilador
clean: compilador
	rm compilador
