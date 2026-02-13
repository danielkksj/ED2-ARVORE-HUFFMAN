//DANIEL RODRIGUES ORTIZ
//NICKOLAS ANTUNES 


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

typedef struct reg {
    int simbolo;
    char palavra[30];
    int frequencia;

    Codigo *cod; 
} Reg;

typedef struct huff {
    int simbolo;            
    struct huff *esq;
    struct huff *dir;
} Huff;


Huff *novoNo() {
    Huff *n = (Huff*)malloc(sizeof(Huff));
    n->simbolo = -1;
    n->esq = n->dir = NULL;
    return n;
}


void inserirCodigoNaArvore(Huff **raiz, Codigo *cod, int simbolo) {
	
	if (*raiz == NULL)
        *raiz = novoNo();	
    int i;
    Huff *atual = *raiz;
    Codigo *aux = cod;

    while (aux != NULL) {

        Byte b = aux->b;

        int total = aux->usados;

        for ( i = 0; i < total; i++) {
            int bit;

            switch (i) {
                case 0: bit = b.b7; 
						break;
                case 1: bit = b.b6; 
						break;
                case 2: bit = b.b5; 
						break;
                case 3: bit = b.b4; 
						break;
                case 4: bit = b.b3; 
						break;
                case 5: bit = b.b2; 
						break;
                case 6: bit = b.b1; 
						break;
                case 7: bit = b.b0; 
						break;
            }
            if (bit == 0) {
                if (atual->esq == NULL)
                    atual->esq = novoNo();
                atual = atual->esq;
            } else {
                if (atual->dir == NULL)
                    atual->dir = novoNo();
                atual = atual->dir;
            }
        }

        aux = aux->prox;
    }

    atual->simbolo = simbolo;
}

int lerTabela(char *nome, Reg regs[]) {
    FILE *f = fopen(nome, "rb");
    
	unsigned char byte_lido;
    int k, i = 0;
    int nbytes, simbolo, lido;
    int continuar = 1;

    while (continuar) {

        
        lido = fread(&simbolo, sizeof(int), 1, f);

        if (lido != 1) {
            continuar = 0;   
        } else {

            regs[i].simbolo = simbolo;

            fread(regs[i].palavra, sizeof(char), 30, f);
            fread(&regs[i].frequencia, sizeof(int), 1, f);


            fread(&nbytes, sizeof(int), 1, f);

            Codigo *inicio = NULL;
            Codigo *ultimo = NULL;

            k = 0;
            while (k < nbytes) {

                
                fread(&byte_lido, sizeof(unsigned char), 1, f);

                Codigo *novo = (Codigo*)malloc(sizeof(Codigo));
                novo->prox = NULL;
                novo->usados = 8;

                novo->b.b7 = (byte_lido >> 7) & 1;
                novo->b.b6 = (byte_lido >> 6) & 1;
                novo->b.b5 = (byte_lido >> 5) & 1;
                novo->b.b4 = (byte_lido >> 4) & 1;
                novo->b.b3 = (byte_lido >> 3) & 1;
                novo->b.b2 = (byte_lido >> 2) & 1;
                novo->b.b1 = (byte_lido >> 1) & 1;
                novo->b.b0 = (byte_lido) & 1;

                if (inicio == NULL)
                    inicio = novo;
                else
                    ultimo->prox = novo;

                ultimo = novo;

                k++;
            }

            regs[i].cod = inicio;

            i++;
        }
    }

    fclose(f);
    return i;
}



unsigned char *lerCompactado(char *nome, int *total_bytes) {
    FILE *f = fopen(nome, "rb");

    fread(total_bytes, sizeof(int), 1, f);

    unsigned char *buf = (unsigned char*)malloc(*total_bytes);
    fread(buf, 1, *total_bytes, f);

    fclose(f);
    return buf;
}


void decodificar(Huff *raiz, unsigned char *buf, int total_bytes, Reg regs[], int nRegs) {
    char frase[5000] = "";
    int k, b, i, sim, flag, bit;
    unsigned char byte;
    Huff *atual = raiz;

    for (b = 0; b < total_bytes; b++) {
    	byte = buf[b];

        for (i = 7; i >= 0; i--) {
         	bit = (byte >> i) & 1;

            if (bit == 0) 
				atual = atual->esq;
            else     
				atual = atual->dir;

            if (atual->esq == NULL && atual->dir == NULL) {
                sim = atual->simbolo;
                flag = 0;  
                k = 0;
                
                while (k < nRegs && flag == 0) {
                    if (regs[k].simbolo == sim) {
                        if (strcmp(regs[k].palavra, " ") == 0)
                            strcat(frase, " ");
                        else
                            strcat(frase, regs[k].palavra);
                        
						flag = 1;   
                    }
                    k++;
                }
                atual = raiz;
            }
        }
    }
    printf("\nFRASE DECODIFICADA:\n%s\n", frase);
}



int main() {
	
    Reg regs[300];
    int i, nregs = lerTabela("Tabela.dat", regs);
    int total_bytes;
    Huff *arvore = NULL;
    unsigned char *frase = lerCompactado("compactado.dat", &total_bytes);
    
    printf("%d registros carregados da tabela.\n", nregs);
    for (i = 0; i < nregs; i++) 
    	inserirCodigoNaArvore(&arvore, regs[i].cod, regs[i].simbolo);

    decodificar(arvore, frase, total_bytes, regs, nregs);

    free(frase);
    return 0;
}






