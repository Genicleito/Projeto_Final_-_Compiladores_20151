%{

/*
* Analisador Sintático (Parser) do compilador
* Disciplina: MATA61 - Compiladores - 2015.1
* Dicentes: Genicleito e Wilton
* Docente: Vinicius Petrucci
*/

/*
* 	Esse programa deve receber uma lista de tokens da análise léxica, deve realizar a análise sintática/semântica e retornar ao seu final a
* árvore sintática do programa compilado, apontando erros consistentes de sintaxe, tais como: palavras escritas erradas, ou instruções
* utilizadas de forma indevida, desta forma também, definindo o fluxo de execução do programa.
* 	Neste programa há uma referência para "symtab.h", que é uma lista com os identificadores do código fonte escrito. Através dela pode
* ser feito uma análise semântica desses identificadores, como por exemplo: IDs usados e não declarados.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>	/* Esta biblioteca será necessária porque tem operadores que podem exigir mais parâmetros, desse modo, não podemos estipular/quantificar um valor da quantidade de parâmetros que sirva para todos */
#include "util.h"	/* Importação do header dos nós (informações como o tipo e valor, por exemplo) */
#include <string.h>
#include "symtab.h"	/* Tabela de símbolos */

/* Prototipação dos tipos de nós que serão usados na árvore sintática */
nodeType *opr(int oper, int nops, ...);
nodeType *id(char *i);
nodeType *con(int value);
void freeNode(nodeType *p);
int ex(nodeType *p);
int yylex(void);

void yyerror(char *s);

int cLinhas;	/* Variável criada no "util.h" e referenciada para uso externo. Este é um contador do número de linhas de código */

%}

/* Definição para os tipos dos tokens lidos e para os tipos dos nós */
%union {
    int iValue;                 /* Valor inteiro para CONSTANTES */
    char sIndex[100];                /* Index dos IDs */
    nodeType *nPtr;             /* Ponteiro para o Nó */
};

/* Definição dos tipos de tokens que foram passados pelo analisador léxico, além de indicar a precedência e associatividade dos tokens em questão */
%token <iValue> INT
%token <sIndex> ID
%token WHILE IF PRINT THEN DO NOT VAR AND OR END
%nonassoc IFX
%nonassoc ELSE

%left GE LE EQ NE '>' '<'
%left '+' '-'
%left '*' '/'
%nonassoc UMINUS NOT

/* Definição dos tipos de alguns nós que serão lidos nesta análise sintática */
%type <nPtr> stmt expr stmts Stmt parteElse

%%

/* Regra de Inicio da análise sintática */
bloco:
        comando                { exit(0); }
        ;

/* Esta regra é para gerar um ou mais stmts no programa, ou seja, uma ou mais instruções */
comando:
          comando stmt       { ex($2); freeNode($2); }		/* A função ex() (presente no arquivo cgen.c) irá gerar código para as regras e tokens lidos a seguir */
        | /* NULL */
        ;

/* Essas são as instruções do programa que irão gerar a árvore, em geral os nós são criados a partir do tipo do nó, da quantidade de argumentos ou parâmetros e a referência dos parâmetros
*/
stmt:
          ';'                      { $$ = opr(';', 2, NULL, NULL); }
        | expr                       { $$ = $1; }
        | PRINT '(' expr ')'                 { $$ = opr(PRINT, 1, $3); }	/* A única função da linguagem é o print(expr) */
/* Abaixo são casos em que uma ID está sendo inicializada, com valor definido ou não, nesse caso dá erro somente se não tiver espaço na memória para alocação */
	| VAR ID		{ if (incluiId($2)) $$ = id($2); else yyerror("erro de memoria"); }
        | VAR ID '=' expr          { if (incluiId($2)) $$ = opr('=', 2, id($2), $4); else yyerror("erro de memoria"); }
/* No caso do ID abaixo há uma checagem na tabela de simbolos para saber se ele foi declarado, retornando erro caso contrário */
	| ID '=' expr	{ if (validar($1)) $$ = opr('=', 2, id($1), $3); else { printf(">> Erro Semantico [Linha %d]:\n\tID '%s' nao declarada\n", cLinhas - 1, $1); exit(1); } }
        | WHILE '(' expr ')' DO Stmt        { $$ = opr(WHILE, 2, $3, $6); }
        | IF '(' expr ')' THEN Stmt %prec IFX { $$ = opr(IF, 2, $3, $6); }
        | IF '(' expr ')' parteElse ELSE Stmt { $$ = opr(IF, 3, $3, $5, $7); }
        ;

/* Regra para permitir que o WHILE tenha uma ou várias instruções em seu escopo */
Stmt:
	stmts END	{ $$ = $1; }
	;

/* Regra para permitir que o ELSE tenha uma ou várias instruções em seu escopo */
parteElse:
	THEN stmts	{ $$ = $2; }
	;

/* Essa regra é utilizada durante os loops para que possam ter uma ou mais instruções dentro do loop */
stmts:
          stmt                  { $$ = $1; }
        | stmts stmt        { $$ = opr(';', 2, $1, $2); }
        ;

/* Essas são as regras para expressões numéricas, seja com operações aritméticas e/ou lógicas, tanto os operadores do tipo binário como: =, +, etc, como do tipo unário: NOT, - */
expr:
          INT               { $$ = con($1); }
/* No caso do ID abaixo há uma checagem na tabela de simbolos para saber se ele foi declarado, retornando erro caso contrário */
        | ID              { if(validar($1)) $$ = id($1); else { printf(">> Erro Semantico [Linha %d]:\n\tID '%s' nao declarada\n", cLinhas, $1); exit(1); } }
        | '-' expr %prec UMINUS { $$ = opr(UMINUS, 1, $2); }
        | NOT expr %prec NOT	{ $$ = opr(NOT, 1, $2); }
        | expr '+' expr         { $$ = opr('+', 2, $1, $3); }
        | expr '-' expr         { $$ = opr('-', 2, $1, $3); }
        | expr '*' expr         { $$ = opr('*', 2, $1, $3); }
        | expr '/' expr         { $$ = opr('/', 2, $1, $3); }
        | expr '<' expr         { $$ = opr('<', 2, $1, $3); }
        | expr '>' expr         { $$ = opr('>', 2, $1, $3); }
        | expr GE expr          { $$ = opr(GE, 2, $1, $3); }
        | expr LE expr          { $$ = opr(LE, 2, $1, $3); }
        | expr NE expr          { $$ = opr(NE, 2, $1, $3); }
        | expr EQ expr          { $$ = opr(EQ, 2, $1, $3); }
        | '(' expr ')'          { $$ = $2; }
	| '(' expr ')' AND '('expr ')' {$$ = opr(AND, 2, $2, $6);}
	| '(' expr ')' OR '(' expr ')' {$$ = opr(OR, 2, $2, $6);}
        ;

%%

/* Alocação e linkagem de nó do tipo constante */
nodeType *con(int value) {
    nodeType *p;

    /* Alocar nó */
    if ((p = malloc(sizeof(nodeType))) == NULL)
        yyerror("erro de memoria");

    /* Copiar Informação */
    p->type = typeCon;
    p->con.value = value;

    return p;
}

/* Alocação e linkagem de nó do tipo identificador */
nodeType *id(char *i) {
    nodeType *p;

    /* Alocar nó */
    if ((p = malloc(sizeof(nodeType))) == NULL)
        yyerror("erro de memoria");

    /* Copiar Informação */
    p->type = typeId;
    strcpy(p->id.id, i);

    return p;
}

/* Alocação e definição dos outros tipos de nó. Nesta parte utilizamos a biblioteca stdarg.h para a criação dos nós (há referência no relatório) */
nodeType *opr(int oper, int nops, ...) {
    va_list ap;
    nodeType *p;
    int i;

    /* Alocar nó */
    if ((p = malloc(sizeof(nodeType))) == NULL)
        yyerror("erro de memoria");
    if ((p->opr.op = malloc(nops * sizeof(nodeType *))) == NULL)
        yyerror("erro de memoria");

    /* Copiar Informação */
    p->type = typeOpr;
    p->opr.oper = oper;
    p->opr.nops = nops;
    va_start(ap, nops);		/* Função presente na biblioteca stdarg.h. Começa a iteração com um va_list (INICIA) */
    for (i = 0; i < nops; i++)
        p->opr.op[i] = va_arg(ap, nodeType*);	/* va_arg: recupera um argumento */
    va_end(ap);		/* Limpa uma va_list (ENCERRA) */
    return p;
}

/* Limpar os nós ao terminar a análise */
void freeNode(nodeType *p) {
    int i;

    if (!p) return;
    if (p->type == typeOpr) {
        for (i = 0; i < p->opr.nops; i++)
            freeNode(p->opr.op[i]);
		free (p->opr.op);
    }
    free (p);
}

/* Função do yacc/bison para reportar erro durante a análise. Note que a linha do erro é indicada. */
void yyerror(char *s) {
	fprintf(stdout, "%s na linha %d\n", s, cLinhas);
}

/* Função main que inicia a análise sintática */
int main(void) {
    yyparse();
    return 0;
}
