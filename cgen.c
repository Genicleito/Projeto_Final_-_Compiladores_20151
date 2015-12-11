#include <stdio.h>
#include "util.h"
#include "y.tab.h"
#include <string.h>

static int lbl;		/* Label para ser usado em condições */
int cont = 0, v[50], j = 0, vW = 0, vOR = 0, vAND = 0, lblAux = 0, vNOT = 0;
/* Descrição das variáveis acima:
 * int cont: um contador usado apenas na primeira execução da geração de código para adicionar o cabeçalho necessário do mips
 * int v[50]: um vetor que armazena os valores dos (e os resultados) dos operandos em uma operação binária
 * int j: marcador do final do vetor citado acima
 * int vW: variável utilizada para validar o uso ou não do comando WHILE e evitar problemas como o laço infinito
 * */
FILE *fOutput;
char c[50];

int ex(nodeType *p) {
    int lbl1, lbl2;		/* Labels para serem utilizados nas condições do IF e WHILE */

    if (!p) return 0;
    
    /* Para inserir um cabeçalho no arquivo de saida <output.asm> e criar um ponteiro para este arquivo de saída */
    if( cont == 0){
		fOutput = fopen("output.asm", "w");
		fprintf(fOutput, "# Trabalho final - Compiladores - 2015.1\n");
		fprintf(fOutput, ".text\n");
		fprintf(fOutput, ".globl  main\n");
		fprintf(fOutput, "main:\n");
		cont++;
	}
    
    /* Verificar o tipo do token lido */
    switch(p->type) {
    case typeCon:		/* Gerar código para números inteiros constantes	*/
		fprintf(fOutput, "\tli\t$a0,\t%d\n", p->con.value);
        fprintf(fOutput, "\tsw\t$a0,\t0($sp)\n");
        fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t-4\n");
        fprintf(fOutput, "#push\t%d\n", p->con.value); 
		v[j] = p->con.value;		/* O valor da constante é armazenado no vetor de operandos */
		j++;
        break;
    case typeId:	/* Gerar código para um identificador	*/
		if(vW > 0 && strcmp(c, p->id.id) == 0){		/* Faz essa verificação caso tenha um while ativo, para evitar loop infinito */
			fprintf(fOutput, "\tsw\t$a0,\t0($sp)\n");
			fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t-4\n");
			vW++;
			break;
		}
		
		/* getValor(p->id.id) está presente no symtab.h e retorna o valor do id passado como parametro */
		fprintf(fOutput, "\tli\t$a0,\t%d\n", getValor(p->id.id));
		fprintf(fOutput, "\tsw\t$a0,\t0($sp)\n");
		fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t-4\n");
		fprintf(fOutput, "#push\t%s\t=\t%d\n", p->id.id, getValor(p->id.id));
		
		v[j] = getValor(p->id.id);		/* O valor do identificador é armazenado no vetor de operandos */
		j++;
        break;
    case typeOpr:		/* Caso encontre um operador */
        switch(p->opr.oper) {
        case WHILE:		/* Gerar código para o WHILE */
			vW = 1;			/* Ativado o marcado que informa que tem um WHILE ativo, para evitar loop infinito */
			nodeType *pAux = p->opr.op[0];
			pAux = pAux->opr.op[0];
			
			/* Verificação para não entrar em loop no while quando usado uma variável pra valida-lo */
			if(pAux->type == typeId){	/* Aqui busco qual é o identificador de controle antes de iniciar o escopo do WHILE */
				fprintf(fOutput, "\tli\t$a0,\t%d\n", getValor(pAux->id.id));
				fprintf(fOutput, "\tsw\t$a0,\t0($sp)\n");
				fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t-4\n");
				strcpy(c, pAux->id.id);
			}

            fprintf(fOutput, "L%03d:\n", lbl1 = lbl++);
            ex(p->opr.op[0]);
            fprintf(fOutput, "\tL%03d\n", lbl2 = lbl++);
            ex(p->opr.op[1]);
            fprintf(fOutput, "\tb\tL%03d\n", lbl1);
            fprintf(fOutput, "L%03d:\n", lbl2);
            vW = 0;
            break;
        case IF:		/* Gerar código para IF */
            ex(p->opr.op[0]);
            if (p->opr.nops > 2) {
                /* if else */
                fprintf(fOutput, "\tL%03d\n", lbl1 = lbl++);
                ex(p->opr.op[1]);
                fprintf(fOutput, "\tb\tL%03d\n", lbl2 = lbl++);
                fprintf(fOutput, "L%03d:\n", lbl1);
                ex(p->opr.op[2]);
                fprintf(fOutput, "L%03d:\n", lbl2);
            } else {
                /* if */
                fprintf(fOutput, "\tL%03d\n", lbl1 = lbl++);
                ex(p->opr.op[1]);
                fprintf(fOutput, "L%03d:\n", lbl1);
            }
            break;
		case PRINT:		/* Gerar código para a função PRINT */
			ex(p->opr.op[0]);
			fprintf(fOutput, "\tli\t$v0,\t1\t\n");
			fprintf(fOutput, "\tsyscall\n");
			fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t4\n");
			j = 0;
			break;
		case '=':		/* Gerar código para atribuição */
            ex(p->opr.op[1]);
            fprintf(fOutput, "\tlw\t$t1,\t4($sp)\n");
			fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t4\n");
			fprintf(fOutput, "# pop\t%d\tem\t%s\n", v[j - 1], p->opr.op[0]->id.id);
			alterarValor(p->opr.op[0]->id.id, v[j - 1]);		//Armazenar na tabela o valor desempilhado
			j = 0;
            break;
        case UMINUS:	/* Gerar código para negação de uma constante ou variavel */
			ex(p->opr.op[0]);
			fprintf(fOutput, "\tlw\t$t1,\t4($sp)\n");
			fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t4\n");
			fprintf(fOutput, "\tli\t$a0,\t-1\n");
			fprintf(fOutput, "\tmul\t$a0,\t$t1,\t$a0\n");
			fprintf(fOutput, "\tsw\t$a0,\t0($sp)\n");
			fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t-4\n");
			v[j - 1] = v[j - 1] * (-1);
			break;
		case NOT:		/* Gerar código para negação (NOT) de uma operação lógica */
			vNOT++;		/* Variável para marcar o uso de um ou mais negações */
			ex(p->opr.op[0]);
			vNOT--;
			if(vNOT < 1 && (vAND == 0 && vOR == 0)){	/* Caso não esteja negando AND nem de OR */
				fprintf(fOutput, "\tlw\t$a0,\t4($sp)\n");
				fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t4\n");
				fprintf(fOutput, "\tbeq\t$a0,\t$zero,");		//	Aqui recebe o label para executar um else, por exemplo
			} else {
				fprintf(fOutput, "\tlw\t$a0,\t4($sp)\n");
				fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t4\n");
				fprintf(fOutput, "\tbeq\t$a0,\t$zero,\ttrue_NOT%03d\n", lblAux);
				fprintf(fOutput, "\tli\t$a0,\t0\n");
				fprintf(fOutput, "\tsw\t$a0,\t0($sp)\n");
				fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t-4\n");
				fprintf(fOutput, "b\tsair_NOT%03d\n", lblAux);
				fprintf(fOutput, "true_NOT%03d:\n", lblAux);
				fprintf(fOutput, "\tli\t$a0,\t1\n");
				fprintf(fOutput, "\tsw\t$a0,\t0($sp)\n");
				fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t-4\n");
				fprintf(fOutput, "sair_NOT%03d:\n", lblAux);
				lblAux++;
			}
			break;
        default:		/* Chegando aqui, o token pode ser apenas um operador binário */
			if(p->opr.oper == OR)
				vOR++;
			if(p->opr.oper == AND)
				vAND++;
            ex(p->opr.op[0]);
            ex(p->opr.op[1]);
            switch(p->opr.oper) {
            case '+':			/* Gerar código para a soma */
				fprintf(fOutput, "\tlw\t$t1,\t4($sp)\n");
				fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t4\n");
				fprintf(fOutput, "\tlw\t$a0,\t4($sp)\n");
				fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t4\n");
				fprintf(fOutput, "\tadd\t$a0,\t$a0,\t$t1\n");
				fprintf(fOutput, "\tsw\t$a0,\t0($sp)\n");
				fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t-4\n");
				fprintf(fOutput, "# Somar\t$a0\t+\t$t1\n");
				v[j - 2] = v[j - 2] + v[j - 1];		/* Armazena o resultado da soma */
				j--;
				break;
            case '-':		/* Gerar código para a subtração */
				fprintf(fOutput, "\tlw\t$t1,\t4($sp)\n");
				fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t4\n");
				fprintf(fOutput, "\tlw\t$a0,\t4($sp)\n");
				fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t4\n");
				fprintf(fOutput, "\tsub\t$a0,\t$a0,\t$t1\n");
				fprintf(fOutput, "\tsw\t$a0,\t0($sp)\n");
				fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t-4\n");
				fprintf(fOutput, "# Subtrair\t$a0\t-\t$t1\n");
				v[j - 2] = v[j - 2] - v[j - 1];		/* Armazena o resultado da subtração */
				j--;
				break; 
            case '*':		/* Gerar código para a multiplicação */
				fprintf(fOutput, "\tlw\t$t1,\t4($sp)\n");
				fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t4\n");
				fprintf(fOutput, "\tlw\t$a0,\t4($sp)\n");
				fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t4\n");
				fprintf(fOutput, "\tmul\t$a0,\t$a0,\t$t1\n");
				fprintf(fOutput, "\tsw\t$a0,\t0($sp)\n");
				fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t-4\n");
				fprintf(fOutput, "# Multiplicar\t$a0\t-\t$t1\n");
				v[j - 2] = v[j - 2] * v[j - 1];		/* Armazena o resultado da multiplicação */
				j--;
				break;
            case '/':		/* Gerar código para a divisão */
				fprintf(fOutput, "\tlw\t$t1,\t4($sp)\n");
				fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t4\n");
				fprintf(fOutput, "\tlw\t$a0,\t4($sp)\n");
				fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t4\n");
				fprintf(fOutput, "\tdiv\t$a0,\t$a0,\t$t1\n");
				fprintf(fOutput, "\tsw\t$a0,\t0($sp)\n");
				fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t-4\n");
				fprintf(fOutput, "# Dividir\t$a0\t-\t$t1\n");
				v[j - 2] = v[j - 2] / v[j - 1];		/* Armazenar o resultado da divisão */
				j--;
				break;
            case '<':		/* Gerar código para a operador menor */
				fprintf(fOutput, "\tlw\t$t1,\t4($sp)\n");
				fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t4\n");
				fprintf(fOutput, "\tlw\t$a0,\t4($sp)\n");
				fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t4\n");
				if(vOR == 0 && vAND == 0){
					if(vNOT >= 1){
						fprintf(fOutput, "\tblt\t$a0,\t$t1,\tnot_menor%03d\n", lblAux);
						fprintf(fOutput, "\tli\t$a0,\t1\n");
						fprintf(fOutput, "\tsw\t$a0,\t0($sp)\n");
						fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t-4\n");
						fprintf(fOutput, "b\tsair_menor_not%03d\n", lblAux);
						fprintf(fOutput, "not_menor%03d:\n", lblAux);
						fprintf(fOutput, "\tsw\t$zero,\t0($sp)\n");
						fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t-4\n");
						fprintf(fOutput, "sair_menor_not%03d:\n", lblAux);
						lblAux++;
					}else{
						fprintf(fOutput, "\tbgt\t$a0,\t$t1,\tL%03d\n", lbl);
						fprintf(fOutput, "\tbeq\t$a0,\t$t1,");
					}
				}else if(vOR >= 1 || vAND >= 1){
					fprintf(fOutput, "\tbgt\t$a0,\t$t1,\tfalse_menor%03d\n", lblAux);
					fprintf(fOutput, "\tbeq\t$a0,\t$t1,\tfalse_menor%03d\n", lblAux);
					fprintf(fOutput, "\tli\t$a0,\t1\n");
					fprintf(fOutput, "\tsw\t$a0,\t0($sp)\n");
					fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t-4\n");
					fprintf(fOutput, "b\tsair_menor%03d\n", lblAux);
					fprintf(fOutput, "false_menor%03d:\n", lblAux);
					fprintf(fOutput, "\tsw\t$zero,\t0($sp)\n");
					fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t-4\n");
					fprintf(fOutput, "sair_menor%03d:\n", lblAux);
					lblAux++;
				}
				j = 0;
				break;
            case '>':		/* Gerar código para a operador maior */
				fprintf(fOutput, "\tlw\t$t1,\t4($sp)\n");
				fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t4\n");
				fprintf(fOutput, "\tlw\t$a0,\t4($sp)\n");
				fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t4\n");
				if(vOR == 0 && vAND == 0){
					if(vNOT >= 1){
						fprintf(fOutput, "\tbgt\t$a0,\t$t1,\tnot_maior%03d\n", lblAux);
						fprintf(fOutput, "\tli\t$a0,\t1\n");
						fprintf(fOutput, "\tsw\t$a0,\t0($sp)\n");
						fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t-4\n");
						fprintf(fOutput, "b\tsair_maior_not%03d\n", lblAux);
						fprintf(fOutput, "not_maior%03d:\n", lblAux);
						fprintf(fOutput, "\tsw\t$zero,\t0($sp)\n");
						fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t-4\n");
						fprintf(fOutput, "sair_maior_not%03d:\n", lblAux);
						lblAux++;
					}else{
						fprintf(fOutput, "\tbeq\t$a0,\t$t1,\tL%03d\n", lbl);
						fprintf(fOutput, "\tblt\t$a0,\t$t1,");
					}
				}else if(vOR >= 1 || vAND >= 1){
					fprintf(fOutput, "\tbeq\t$a0,\t$t1,\tfalse_maior%03d\n", lblAux);
					fprintf(fOutput, "\tblt\t$a0,\t$t1,\tfalse_maior%03d\n", lblAux);
					fprintf(fOutput, "\tli\t$a0,\t1\n");
					fprintf(fOutput, "\tsw\t$a0,\t0($sp)\n");
					fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t-4\n");
					fprintf(fOutput, "b\tsair_maior%03d\n", lblAux);
					fprintf(fOutput, "false_maior%03d:\n", lblAux);
					fprintf(fOutput, "\tsw\t$zero,\t0($sp)\n");
					fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t-4\n");
					fprintf(fOutput, "sair_maior%03d:\n", lblAux);
					lblAux++;
				}
				j = 0;
				break;
            case GE:		/* Gerar código para a operador maior ou igual */
				fprintf(fOutput, "\tlw\t$t1,\t4($sp)\n");
				fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t4\n");
				fprintf(fOutput, "\tlw\t$a0,\t4($sp)\n");
				fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t4\n");
				if(vOR == 0 && vAND == 0){
					if(vNOT >= 1){
						fprintf(fOutput, "\tbge\t$a0,\t$t1,\tnot_maior_igual%03d\n", lblAux);
						fprintf(fOutput, "\tli\t$a0,\t1\n");
						fprintf(fOutput, "\tsw\t$a0,\t0($sp)\n");
						fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t-4\n");
						fprintf(fOutput, "b\tsair_maior_igual_not%03d\n", lblAux);
						fprintf(fOutput, "not_maior_igual%03d:\n", lblAux);
						fprintf(fOutput, "\tsw\t$zero,\t0($sp)\n");
						fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t-4\n");
						fprintf(fOutput, "sair_maior_igual_not%03d:\n", lblAux);
						lblAux++;
					}else
						fprintf(fOutput, "\tblt\t$a0,\t$t1,");
				}else if(vOR >= 1 || vAND >= 1){
					fprintf(fOutput, "\tblt\t$a0,\t$t1,\tfalse_maior_igual%03d\n", lblAux);
					fprintf(fOutput, "\tli\t$a0,\t1\n");
					fprintf(fOutput, "\tsw\t$a0,\t0($sp)\n");
					fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t-4\n");
					fprintf(fOutput, "b\tsair_maior_igual%03d\n", lblAux);
					fprintf(fOutput, "false_maior_igual%03d:\n", lblAux);
					fprintf(fOutput, "\tsw\t$zero,\t0($sp)\n");
					fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t-4\n");
					fprintf(fOutput, "sair_maior_igual%03d:\n", lblAux);
					lblAux++;
				}
				j = 0;
				break;
            case LE:		/* Gerar código para a operador menor ou igual */
				fprintf(fOutput, "\tlw\t$t1,\t4($sp)\n");
				fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t4\n");
				fprintf(fOutput, "\tlw\t$a0,\t4($sp)\n");
				fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t4\n");
				if(vOR == 0 && vAND == 0){
					if(vNOT >= 1){
						fprintf(fOutput, "\tble\t$a0,\t$t1,\tnot_menor_igual%03d\n", lblAux);
						fprintf(fOutput, "\tli\t$a0,\t1\n");
						fprintf(fOutput, "\tsw\t$a0,\t0($sp)\n");
						fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t-4\n");
						fprintf(fOutput, "b\tsair_menor_igual_not%03d\n", lblAux);
						fprintf(fOutput, "not_menor_igual%03d:\n", lblAux);
						fprintf(fOutput, "\tsw\t$zero,\t0($sp)\n");
						fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t-4\n");
						fprintf(fOutput, "sair_menor_igual_not%03d:\n", lblAux);
						lblAux++;
					}else
						fprintf(fOutput, "\tbgt\t$a0,\t$t1,");
				}else if(vOR >= 1 || vAND >= 1){
					fprintf(fOutput, "\tbgt\t$a0,\t$t1,\tfalse_menor_igual%03d\n", lblAux);
					fprintf(fOutput, "\tli\t$a0,\t1\n");
					fprintf(fOutput, "\tsw\t$a0,\t0($sp)\n");
					fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t-4\n");
					fprintf(fOutput, "b\tsair_menor_igual%03d\n", lblAux);
					fprintf(fOutput, "false_menor_igual%03d:\n", lblAux);
					fprintf(fOutput, "\tsw\t$zero,\t0($sp)\n");
					fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t-4\n");
					fprintf(fOutput, "sair_menor_igual%03d:\n", lblAux);
					lblAux++;
				}
				j = 0;
				break;
            case NE: 		/* Gerar código para a operador diferente (não-igual) */
				fprintf(fOutput, "\tlw\t$t1,\t4($sp)\n");
				fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t4\n");
				fprintf(fOutput, "\tlw\t$a0,\t4($sp)\n");
				fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t4\n");
				if(vOR == 0 && vAND == 0){
					if(vNOT >= 1){
						fprintf(fOutput, "\tbne\t$a0,\t$t1,\tnot_NE%03d\n", lblAux);
						fprintf(fOutput, "\tli\t$a0,\t1\n");
						fprintf(fOutput, "\tsw\t$a0,\t0($sp)\n");
						fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t-4\n");
						fprintf(fOutput, "b\tsair_NE_not%03d\n", lblAux);
						fprintf(fOutput, "not_NE%03d:\n", lblAux);
						fprintf(fOutput, "\tsw\t$zero,\t0($sp)\n");
						fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t-4\n");
						fprintf(fOutput, "sair_NE_not%03d:\n", lblAux);
						lblAux++;
					}else
						fprintf(fOutput, "\tbeq\t$a0,\t$t1,");
				}else if(vOR >= 1 || vAND >= 1){
					fprintf(fOutput, "\tbeq\t$a0,\t$t1,\tfalse_diferente%03d\n", lblAux);
					fprintf(fOutput, "\tli\t$a0,\t1\n");
					fprintf(fOutput, "\tsw\t$a0,\t0($sp)\n");
					fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t-4\n");
					fprintf(fOutput, "b\tsair_diferente%03d\n", lblAux);
					fprintf(fOutput, "false_diferente%03d:\n", lblAux);
					fprintf(fOutput, "\tsw\t$zero,\t0($sp)\n");
					fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t-4\n");
					fprintf(fOutput, "sair_diferente%03d:\n", lblAux);
					lblAux++;
				}
				j = 0;
				break;
            case EQ:		/* Gerar código para a operador igual */
				fprintf(fOutput, "\tlw\t$t1,\t4($sp)\n");
				fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t4\n");
				fprintf(fOutput, "\tlw\t$a0,\t4($sp)\n");
				fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t4\n");
				if(vOR == 0 && vAND == 0){
					if(vNOT >= 1){
						fprintf(fOutput, "\tbeq\t$a0,\t$t1,\tnot_EQ%03d\n", lblAux);
						fprintf(fOutput, "\tli\t$a0,\t1\n");
						fprintf(fOutput, "\tsw\t$a0,\t0($sp)\n");
						fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t-4\n");
						fprintf(fOutput, "b\tsair_EQ_not%03d\n", lblAux);
						fprintf(fOutput, "not_EQ%03d:\n", lblAux);
						fprintf(fOutput, "\tsw\t$zero,\t0($sp)\n");
						fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t-4\n");
						fprintf(fOutput, "sair_EQ_not%03d:\n", lblAux);
						lblAux++;
					}else
						fprintf(fOutput, "\tbne\t$a0,\t$t1,");
				}else if(vOR >= 1 || vAND >= 1){
					fprintf(fOutput, "\tbne\t$a0,\t$t1,\tfalse_igual%03d\n", lblAux);
					fprintf(fOutput, "\tli\t$a0,\t1\n");
					fprintf(fOutput, "\tsw\t$a0,\t0($sp)\n");
					fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t-4\n");
					fprintf(fOutput, "b\tsair_igual%03d\n", lblAux);
					fprintf(fOutput, "false_igual%03d:\n", lblAux);
					fprintf(fOutput, "\tsw\t$zero,\t0($sp)\n");
					fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t-4\n");
					fprintf(fOutput, "sair_igual%03d:\n", lblAux);
					lblAux++;
				}
				j = 0;
				break;
			case OR:		/* Gerar código para a operador lógico "ou" (OR) */
				fprintf(fOutput, "\tlw\t$t1,\t4($sp)\n");
				fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t4\n");
				fprintf(fOutput, "\tlw\t$a0,\t4($sp)\n");
				fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t4\n");
				fprintf(fOutput, "\tor\t$a0,\t$t1,\t$a0\n");
				if(vOR <= 1 && vAND == 0){
					if(vNOT == 0)
						fprintf(fOutput, "\tbeq\t$a0,\t$zero,");		//	Caso sejam falsas as comparações do OR, aqui recebe o label para executar um else, por exemplo
					else{
						fprintf(fOutput, "\tli\t$t1,\t1\n");
						fprintf(fOutput, "\tbeq\t$a0,\t$t1,\tnot_or%03d\n", lblAux);
						fprintf(fOutput, "\tli\t$a0,\t1\n");
						fprintf(fOutput, "\tsw\t$a0,\t0($sp)\n");
						fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t-4\n");
						fprintf(fOutput, "b\tsair_igual%03d\n", lblAux);
						fprintf(fOutput, "not_or%03d:\n", lblAux);
						fprintf(fOutput, "\tsw\t$zero,\t0($sp)\n");
						fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t-4\n");
						fprintf(fOutput, "sair_igual%03d:\n", lblAux);
						lblAux++;
					}
				}else{
					fprintf(fOutput, "\tsw\t$a0,\t0($sp)\n");
					fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t-4\n");
				}
				if(vOR != 0)
					vOR--;
				j = 0;
				break;
			case AND:		/* Gerar código para a operador lógico "e" (AND) */
				fprintf(fOutput, "\tlw\t$t1,\t4($sp)\n");
				fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t4\n");
				fprintf(fOutput, "\tlw\t$a0,\t4($sp)\n");
				fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t4\n");
				fprintf(fOutput, "\tand\t$a0,\t$t1,\t$a0\n");
				if(vAND <= 1 && vOR == 0){
					if(vNOT == 0)
						fprintf(fOutput, "\tbeq\t$a0,\t$zero,");		//	Caso sejam falsas as comparações do AND, aqui recebe o label para executar um else, por exemplo
					else{
						fprintf(fOutput, "\tli\t$t1,\t1\n");
						fprintf(fOutput, "\tbeq\t$a0,\t$t1,\tnot_or%03d\n", lblAux);
						fprintf(fOutput, "\tli\t$a0,\t1\n");
						fprintf(fOutput, "\tsw\t$a0,\t0($sp)\n");
						fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t-4\n");
						fprintf(fOutput, "b\tsair_igual%03d\n", lblAux);
						fprintf(fOutput, "not_or%03d:\n", lblAux);
						fprintf(fOutput, "\tsw\t$zero,\t0($sp)\n");
						fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t-4\n");
						fprintf(fOutput, "sair_igual%03d:\n", lblAux);
						lblAux++;
					}
				}else{
					fprintf(fOutput, "\tsw\t$a0,\t0($sp)\n");
					fprintf(fOutput, "\taddiu\t$sp,\t$sp,\t-4\n");
				}
				if(vAND != 0)
					vAND--;
				j = 0;
				break;
            }
        }
    }
    return 0;
}
