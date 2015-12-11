/*
* Header de definição das estruturas
* Disciplina: MATA61 - Compiladores - 2015.1
* Dicentes: Genicleito e Wilton
* Docente: Vinicius Petrucci
*/

/* Esse header traz a implementação e definição de todos os tipos de nós que serão utilizados durante
a geração da árvore sintática no parser, tal qual também inclui a tabela de simbolos, que será usada
durante todo o processo de compilação */

/* Serve para indexar os tipos de nós em uma enumeração */
typedef enum { typeCon, typeId, typeOpr } nodeEnum;

/* Nó para Constantes.
Esse tipo de nó é usado para constantes inteiras no programa que não precisam ser inicializadas. */
typedef struct {
    int value;                  /* value of constant */
} conNodeType;

/* Nó para Identificadores.
Esse tipo de nó é utilizado para guardar variáveis e seu valor */
typedef struct {
    char id[100];
} idNodeType;

/* Nó para operadores, como =, +, -, etc
Nó utilizado para gerar sub-árvores binárias de operações elementares (atribuição, soma, ...) */
typedef struct {
    int oper;                   /* Operador, número de acordo com a ASCII */
    int nops;                   /* Quantidade de operandos */
    struct nodeTypeTag **op;	/* Operandos */
} oprNodeType;

/* Essa será a estrutura pela qual será utilizada no parser, como macro para utilização dos tipos de nós */
typedef struct nodeTypeTag {
    nodeEnum type;              /* Tipo do nó */

    union {
        conNodeType con;        /* Constantes */
        idNodeType id;          /* Identificadores */
        oprNodeType opr;        /* Operadores */
    };
} nodeType;

/* Variável de uso externo usada como contador de linhas na análise léxica e sintática */
extern int cLinhas;
