%{

/*
* Analisador Léxico
* Disciplina: MATA61 - Compiladores - 2015.1
* Discentes: Genicleito e Wilton
* Docente: Vinicius Petrucci
*/

/*
	Esse programa deve realizar a análise léxica, e para cada palavra processada deve retornar
o tipo do respectivo token ou retornar erro léxico se não contiver nas definições da gramática da linguagem.
	A saída desta análise é uma lista de tokens que será utilizada no parser (bison) para geração da árvore sintática
e para validação das regras da gramática através da integração do flex/bison.
*/

#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "y.tab.h"

void yyerror(char *);
int cLinhas = 1;	// Variável contadora de número de linhas

%}

/* Regra para Identificadores */
id [_a-zA-Z][a-zA-Z0-9_]*

%%

">="            return GE;	/* Definição de Operadores Lógicos e Palavras Reservadas da Linguagem */
"<="            return LE;
"=="            return EQ;
"~="            return NE;
"while"         return WHILE;
"if"            return IF;
"else"          return ELSE;
"print"         return PRINT;
"do"		return DO;
"and"		return AND;
"or"		return OR;
"end"		return END;
"then"		return THEN;
"not"		return NOT;
"var"		return VAR;

[ \t]+	;	/* Ignorar espaços em branco e tabs */

[\n]+	{	/* Ao encontrar um fim de linha, incrementamos o contador de linhas */
		cLinhas++;
	}

{id}	{	/* Identifica um identificador e retorna o token ID */
		strcpy(yylval.sIndex, yytext);
		return ID;
	}

[0-9]+	{	/* Identifica um Inteiro e retorna o token INT */
		yylval.iValue = atoi(yytext);
		return INT;
	}

[-()<>=+*/;,]	{	/* Identifica um Operador e retorna o respectivo Token */
			return *yytext;
		}

.		{ printf(">> Erro lexico '%s' na linha %d\n", yytext, cLinhas); exit(1); }	/* Caso não entre em nenhuma das regras anteriores */

%%
int yywrap(void) {
    return 1;
}
