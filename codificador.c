//DANIEL RODRIGUES ORTIZ
//NICKOLAS ANTUNES 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio2.h>

typedef struct no {
    char palavra[30];
    int freq;
    struct no *prox;
} No;

typedef struct bits {
    unsigned char b7:1;
    unsigned char b6:1;
    unsigned char b5:1;
    unsigned char b4:1;
    unsigned char b3:1;
    unsigned char b2:1;
    unsigned char b1:1;
    unsigned char b0:1;
} Byte;

typedef struct codigo {
    Byte b;
    int usados;
    struct codigo *prox;
} Codigo;

typedef struct huff {
    int simbolo;
    int freq;
    struct huff *esq;
    struct huff *dir;
} Huff;

typedef struct lista {
    Huff *no;
    struct lista *prox;
} Lista;

typedef struct reg {
    int simbolo;
    char palavra[30];
    int frequencia;
    Codigo *cod;   
} Reg;

Byte zerarByte() {
    Byte x;
    x.b7 = x.b6 = x.b5 = x.b4 = x.b3 = x.b2 = x.b1 = x.b0 = 0;
    return x;
}

Codigo* novoCodigo() {
    Codigo *c = (Codigo*)malloc(sizeof(Codigo));
    c->b = zerarByte();
    c->usados = 0;
    c->prox = NULL;
    return c;
}

void adicionarBit(Codigo **cod, int bit) {
    Codigo *aux = *cod;
    while (aux->prox != NULL) {
        aux = aux->prox;
    }

    if (aux->usados == 8) {
        aux->prox = novoCodigo();
        aux = aux->prox;
    }

    switch (aux->usados) {
        case 0: aux->b.b7 = bit; 
			break;
        case 1: aux->b.b6 = bit; 
			break;
        case 2: aux->b.b5 = bit; 
			break;
        case 3: aux->b.b4 = bit; 
			break;
        case 4: aux->b.b3 = bit; 
			break;
        case 5: aux->b.b2 = bit; 
			break;
        case 6: aux->b.b1 = bit;
			break;
        case 7: aux->b.b0 = bit; 
			break;
    }

    aux->usados++;
}

void normalizarTexto(char *s) {
    int i = 0;
    while (s[i] != '\0') {
        if (s[i] >= 'A' && s[i] <= 'Z') {
            s[i] = s[i] - 'A' + 'a';
        }
        i++;
    }
}

void inserirLista(No **lista, char *pal) {
    No *aux = *lista;
    char flag = 0;
    
    while (aux != NULL && flag == 0) {
        if (strcmp(aux->palavra, pal) == 0) {
            aux->freq++;
            flag = 1;
        } else {
            aux = aux->prox;
        }
    }
    
    if (flag == 0) {
        No *novo = (No*)malloc(sizeof(No));
        strcpy(novo->palavra, pal);
        novo->freq = 1;
        novo->prox = *lista;
        *lista = novo;  
    }
}

void processar_texto(char *texto, No **lista) {
    char palavra[30];
    int i = 0, j = 0;
    normalizarTexto(texto);
    
    while (texto[i] != '\0') {
        if (texto[i] != ' ') {
            palavra[j++] = texto[i];
        } else {
            if (j > 0) {
                palavra[j] = '\0';
                inserirLista(lista, palavra);
                j = 0;
            }
            inserirLista(lista, " ");
        }
        i++;
    }
    if (j > 0) {
        palavra[j] = '\0';
        inserirLista(lista, palavra);
    }
}

void inserir_ordenado_lista(Lista **L, Huff *h) {
    Lista *novo = (Lista*)malloc(sizeof(Lista));
    novo->no = h;
    novo->prox = NULL;
    Lista *aux = *L;
    Lista *ant = NULL;
    
    while (aux != NULL && aux->no->freq <= h->freq) {
        ant = aux;
        aux = aux->prox;
    }
    
    novo->prox = aux;
    if (ant == NULL) {
        *L = novo;
    } else {
        ant->prox = novo;
    }
}

Lista *converter_para_lista_huff(No *lista, Reg tabela[]) {
    Lista *L = NULL;
    int simbolo = 0;
    while (lista != NULL) {
        Huff *novo = (Huff*)malloc(sizeof(Huff));
        novo->simbolo = simbolo;
        novo->freq = lista->freq;
        novo->esq = NULL;
        novo->dir = NULL;
        tabela[simbolo].simbolo = simbolo;
        tabela[simbolo].frequencia = lista->freq;
        tabela[simbolo].cod = NULL;
        strcpy(tabela[simbolo].palavra, lista->palavra);
        inserir_ordenado_lista(&L, novo);
        simbolo++;
        lista = lista->prox;
    }
    tabela[simbolo].palavra[0] = '\0';
    return L;
}

Huff *montar_arvore(Lista **L) {
    while (*L != NULL && (*L)->prox != NULL) {
        Lista *a = *L;
        Lista *b = a->prox;
        Lista *resto = b->prox;
        Huff *noA = a->no;
        Huff *noB = b->no;
        Huff *pai = (Huff*)malloc(sizeof(Huff));
        pai->simbolo = -1;
        pai->freq = noA->freq + noB->freq;
        pai->esq = noA;
        pai->dir = noB;
        free(a);
        free(b);
        *L = resto;
        inserir_ordenado_lista(L, pai);
    }
    Huff *raiz = (*L)->no;
    free(*L);
    *L = NULL;
    return raiz;
}

Codigo *copiarCodigo(Codigo *orig) {
    if (orig == NULL) return NULL;
    Codigo *novo = novoCodigo();
    novo->b = orig->b;
    novo->usados = orig->usados;
    Codigo *auxN = novo;
    Codigo *auxO = orig->prox;
    while (auxO != NULL) {
        auxN->prox = novoCodigo();
        auxN = auxN->prox;
        auxN->b = auxO->b;
        auxN->usados = auxO->usados;
        auxO = auxO->prox;
    }
    return novo;
}

void gerar_codigos(Huff *raiz, Codigo *prefixo, Codigo **tabela) {
    Codigo *copia1, *copia0;
    if(raiz != NULL) {
        if (raiz->esq == NULL && raiz->dir == NULL) {
            tabela[raiz->simbolo] = prefixo;
        } else {
            copia1 = copiarCodigo(prefixo);
            adicionarBit(&copia1, 1);
            gerar_codigos(raiz->dir, copia1, tabela);
            copia0 = copiarCodigo(prefixo);
            adicionarBit(&copia0, 0);
            gerar_codigos(raiz->esq, copia0, tabela);
        }
    }
}

void gerarBin(Reg tabela[], char *nome_arquivo) {
    FILE *f = fopen(nome_arquivo, "wb");
    int i = 0;
    while (tabela[i].palavra[0] != '\0') {
        fwrite(&tabela[i].simbolo, sizeof(int), 1, f);
        fwrite(tabela[i].palavra, sizeof(char), 30, f);
        fwrite(&tabela[i].frequencia, sizeof(int), 1, f);
        int nbytes = 0;
        Codigo *aux = tabela[i].cod;
        while (aux != NULL) {
            nbytes++;
            aux = aux->prox;
        }
        fwrite(&nbytes, sizeof(int), 1, f);
        aux = tabela[i].cod;
        while (aux != NULL) {
            unsigned char byte =
                (aux->b.b7 << 7) | (aux->b.b6 << 6) | (aux->b.b5 << 5) |
                (aux->b.b4 << 4) | (aux->b.b3 << 3) | (aux->b.b2 << 2) |
                (aux->b.b1 << 1) | (aux->b.b0);
            fwrite(&byte, sizeof(unsigned char), 1, f);
            aux = aux->prox;
        }
        i++;
    }
    fclose(f);
}

void anexarCodigo(Codigo **dest, Codigo *src) {
    if (src == NULL) return;
    while (*dest != NULL) {
        dest = &((*dest)->prox);
    }
    Codigo *a = src;
    while (a != NULL) {
        Codigo *novo = (Codigo*)malloc(sizeof(Codigo));
        novo->b = a->b;
        novo->usados = a->usados;
        novo->prox = NULL;
        *dest = novo;
        dest = &((*dest)->prox);
        a = a->prox;
    }
}

int buscarSimbolo(Reg tabela[], char *palavra) {
    int i = 0;
    int flag = 1;
    while (tabela[i].palavra[0] != '\0' && flag == 1) {
        if (strcmp(tabela[i].palavra, palavra) == 0) {
            flag = 0;
        } else {
            i++;
        }
    }
    if(flag == 0) return tabela[i].simbolo;
    return tabela[i-1].simbolo;
}

Codigo *codificar_texto(char *texto, Reg tabela[]) {
    Codigo *codFinal = NULL;
    char palavra[30];
    int i = 0, j = 0;
    normalizarTexto(texto);
    while (texto[i] != '\0') {
        if (texto[i] != ' ') {
            palavra[j++] = texto[i];
        } else {
            if (j > 0) {
                palavra[j] = '\0';
                int sim = buscarSimbolo(tabela, palavra);
                anexarCodigo(&codFinal, tabela[sim].cod);
                j = 0;
            }
            int simEspaco = buscarSimbolo(tabela, " ");
            anexarCodigo(&codFinal, tabela[simEspaco].cod);
        }
        i++;
    }
    if (j > 0) {
        palavra[j] = '\0';
        int sim = buscarSimbolo(tabela, palavra);
        anexarCodigo(&codFinal, tabela[sim].cod);
    }
    return codFinal;
}

void salvar_compactado(Codigo *cod, char *nome) {
    FILE *f = fopen(nome, "wb");
    unsigned char byte;
    int total = 0;
    Codigo *a = cod;
    while (a != NULL) {
        total++;
        a = a->prox;
    }
    fwrite(&total, sizeof(int), 1, f);
    a = cod;
    while (a != NULL) {
        byte =
            (a->b.b7 << 7) | (a->b.b6 << 6) | (a->b.b5 << 5) |
            (a->b.b4 << 4) | (a->b.b3 << 3) | (a->b.b2 << 2) |
            (a->b.b1 << 1) | (a->b.b0);
        fwrite(&byte, sizeof(unsigned char), 1, f);
        a = a->prox;
    }
    fclose(f);
}

void printArvore(Huff *raiz, char *prefixo, int tipoFilho) {
    if (raiz != NULL) {
        
        textcolor(LIGHTBLUE);printf( "%s", prefixo);
        
        if (tipoFilho == 0) {
            printf("|--0-- "); 
        } else if (tipoFilho == 1) {
            printf("\\--1-- "); 
        } else {
            printf("-> "); 
        }

        if (raiz->simbolo == -1) {
            textcolor(LIGHTGRAY);
            printf("(*)"); 
        } else {
            textcolor(YELLOW);
            printf("[%d]", raiz->simbolo); 
        }
        printf("\n"); 

        char novoPrefixo[500];
        strcpy(novoPrefixo, prefixo);
        
        if (tipoFilho == 0) {
            strcat(novoPrefixo, "|      ");
        } else if (tipoFilho == 1) {
            strcat(novoPrefixo, "       ");
        } else {
            strcat(novoPrefixo, "   ");
        }

        printArvore(raiz->esq, novoPrefixo, 0);
        printArvore(raiz->dir, novoPrefixo, 1);
    }
}

int main() {
    textbackground(BLACK);
    clrscr();

    char texto_treino[] =
    "sistema distribuido consiste em varios componentes que se comunicam entre si para trocar informacoes dados e processos cada componente possui funcoes especificas mas todos trabalham em conjunto para manter o sistema operando de forma eficiente quando um componente envia uma solicitacao outro componente responde com os dados necessarios e assim o sistema continua funcionando de maneira sincronizada "
	"arquitetura de software envolve planejamento organizacao e estruturacao de componentes para garantir desempenho seguranca e escalabilidade quando um sistema cresce muito e necessario distribuir funcoes para evitar sobrecarga em um unico ponto tecnologias modernas permitem que componentes sejam replicados distribuidos e balanceados garantindo que o sistema continue a operar mesmo quando algum componente falhar "
	"processamento paralelo e uma tecnica onde varias tarefas sao executadas ao mesmo tempo utilizando diferentes unidades de processamento isso aumenta a velocidade do sistema e permite realizar operacoes complexas em menos tempo alguns sistemas utilizam clusters outros utilizam computacao em nuvem e outros utilizam arquiteturas hibridas dependendo das necessidades da aplicacao "
	"uma rede distribuida precisa lidar com problemas de latencia perda de pacotes e falhas inesperadas por isso protocolos de comunicacao sao desenhados para garantir que mensagens sejam entregues com integridade mesmo em ambientes instaveis repeticao de mensagens verificacao de consistencia e controle de congestionamento sao tecnicas comuns para manter a qualidade da comunicacao "
	"quando falamos de desempenho a cache se torna um elemento importante pois permite salvar temporariamente informacoes acessadas com frequencia reduzindo o tempo de resposta componentes que utilizam cache conseguem fornecer dados rapidamente sem necessidade de consultar a base de dados ou outro servico remoto economizando tempo e recursos "
	"combinando distribuicao processamento paralelo cache e protocolos eficientes criamos sistemas modernos que suportam milhares de usuarios simultaneamente permitindo operacoes confiaveis seguras e escalaveis ";
	
    int i;
    No *lista_palavras = NULL;

    textcolor(WHITE);
    printf("Processando texto...\n");
    processar_texto(texto_treino, &lista_palavras);

    Reg tabela[300];
    for (i = 0; i < 300; i++) tabela[i].palavra[0] = '\0';

    Lista *L = converter_para_lista_huff(lista_palavras, tabela);
    Huff *raiz = montar_arvore(&L);

    Codigo *tabela_codigos[300];
    for (i = 0; i < 300; i++) tabela_codigos[i] = NULL;

    Codigo *prefixoInicial = novoCodigo();
    gerar_codigos(raiz, prefixoInicial, tabela_codigos);
    
    for (i = 0; tabela[i].palavra[0] != '\0'; i++) tabela[i].cod = tabela_codigos[i];

    clrscr();
    textcolor(LIGHTGREEN);
    printf("TABELA DE CODIFICACAO GERADA:\n");
    textcolor(WHITE);
    printf("+--------+--------------------+------------+------------------------------------------+\n");
    printf("| SIMBOLO| PALAVRA            | FREQUENCIA | CODIGO BINARIO                           |\n");
    printf("+--------+--------------------+------------+------------------------------------------+\n");
    
    for (i = 0; tabela[i].palavra[0] != '\0'; i++) {
        printf("| %-6d | %-18.18s | %-10d | ",
               tabela[i].simbolo,
               tabela[i].palavra,
               tabela[i].frequencia);

        Codigo *aux = tabela[i].cod;
        while (aux != NULL) {
            printf("%d%d%d%d%d%d%d%d ",
                aux->b.b7, aux->b.b6, aux->b.b5, aux->b.b4,
                aux->b.b3, aux->b.b2, aux->b.b1, aux->b.b0);
            aux = aux->prox;
        }
        printf("\n");
    }
    printf("+--------+--------------------+------------+------------------------------------------+\n");
    
    printf("\n>>> Pressione ENTER para ver a Arvore de Huffman...");
    getchar();
    
    clrscr();
    gotoxy(1, 1);
    textcolor(LIGHTCYAN);
    printf("=== ARVORE DE HUFFMAN ===\n");
    printf("Legenda: (*) No Interno | (NUM) ID do Simbolo\n\n");
    
    char prefixoBase[500] = "";
    
    printArvore(raiz, prefixoBase, -1);
    
    printf("\n");
    textcolor(WHITE);
    printf(">>> Pressione ENTER para gerar arquivos...");
    getchar();

    printf("\nGerando 'Tabela.dat'... ");
    gerarBin(tabela,"Tabela.dat");
    printf("OK!\n");
    
    char frase[] = "sistema distribuido operando de forma eficiente combinando distribuicao processamento paralelo cache e protocolos";

    printf("Gerando 'compactado.dat'... ");
    Codigo *codificado = codificar_texto(frase, tabela);
    salvar_compactado(codificado, "compactado.dat");
    printf("OK!\n");
    
    textcolor(LIGHTCYAN);
    printf("\nCONCLUIDO.\n");
    getch();

    return 0;
}
