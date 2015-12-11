#include <stdio.h>
#include <stdbool.h>
#include <string.h>

/* Definição dos nós da tabela */
typedef struct TabelaS{
    char identificador[50];
    int valor;
    struct TabelaS *prox;
} TabelaS;

/* Nó raiz */
TabelaS *raiz;

/* Inicializa a tabela */
void inicializaTabela(){
    raiz = NULL;
}

/* Função booleana que dado um identificador retorna NULL se ele não estiver na Tabela */
TabelaS *procura (char *nomeId){
	TabelaS *temp;
	for(temp = raiz; temp != NULL && (strcmp(temp->identificador, nomeId)) != 0; temp = temp->prox);
	return temp;
}

/* Função booleana para incluir uma ID na tabela, ou seja, quando a variável é somente declarada */
bool incluiId (char *nomeId){
	TabelaS *temp;
	if(temp = (TabelaS*)malloc(sizeof(TabelaS))){
		strcpy (temp->identificador, nomeId);
	        temp->valor = 0;
	        temp->prox = (struct TabelaS*) raiz;
	        raiz = temp;
	        return true;
	} else
		return false;
}

/* Função booleana para alterar o valor da variável na tabela */
bool alterarValor (char *nomeId, int val){
    TabelaS *temp;
	temp = procura(nomeId);
	if (!(temp))
        return false;
    else
        temp->valor = val;
    return true;
}

/* Função booleana que verifica se um determinado identificador está ou não na tabela de simbolos */
bool validar (char *nomeId){
	TabelaS *temp;
	temp = procura(nomeId);
	return temp;
}

/* Função inteira que retorna o valor de um identificador presente na tabela de simbolos */
int getValor(char *id){
	TabelaS *temp;
	for(temp = raiz; temp != NULL && (strcmp(temp->identificador, id)) != 0; temp = temp->prox);
	return temp->valor;
}
