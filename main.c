#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "keyboard.h"
#include "screen.h"

#define largura 81
#define altura 24
#define arquivoScore "score.txt"
#define velocidadeInicial 200000
#define velocidadePosmaca 250000
#define corazul "\x1b[34m"
#define corvermelha "\x1b[31m"
#define coramarelo "\x1b[33m"
#define resetarRestante "\x1b[0m"

struct Ponto {
    int x, y;
};

struct CaudaCobra {
    struct Ponto posicao;
    struct CaudaCobra *ptr;
};

struct Cobra {
    struct CaudaCobra *cabeca;
    struct CaudaCobra *cauda;
    int comprimento;
    struct Ponto direcao;
};

struct Comida {
    struct Ponto posicao;
};

void safeScore(int pontuacao) {
    FILE *arquivo = fopen(arquivoScore, "a");
    if (arquivo == NULL) {
        fprintf(stderr, "erro ao abrir o arquivo: %s\n", arquivoScore);
        return;
    }
    fprintf(arquivo, "%d\n", pontuacao);
    fclose(arquivo);
}

void carregarScore() {
    FILE *arquivo = fopen(arquivoScore, "r");
    if (arquivo == NULL) {
        fprintf(stderr, "erro ao abrir o arquivo: %s\n", arquivoScore);
        return;
    }
    int pontuacao;
    while (fscanf(arquivo, "%d", &pontuacao) != EOF) {
        printf("%d ", pontuacao);
    }
    fclose(arquivo);
}


void alocarMemoriaCobra(struct Cobra *cobra) {
    struct CaudaCobra *corpo = cobra->cabeca;
    while (corpo != NULL) {
        struct CaudaCobra *ptr = corpo->ptr;
        free(corpo);  
        corpo = ptr;
    }
    cobra->cabeca = NULL;
    cobra->cauda = NULL;
}

void inicio(struct Cobra *cobra, struct Comida *comida, int *velocidade) {
    struct CaudaCobra *cabecaCauda = (struct CaudaCobra *)malloc(sizeof(struct CaudaCobra));
    cabecaCauda->posicao.x = largura / 2;
    cabecaCauda->posicao.y = altura / 2;
    cabecaCauda->ptr = NULL;
    cobra->cabeca = cabecaCauda;
    cobra->cauda = cabecaCauda;  
    cobra->comprimento = 1;
    cobra->direcao.x = 1;
    cobra->direcao.y = 0;
    comida->posicao.x = rand() % largura;
    comida->posicao.y = rand() % altura;
    *velocidade = velocidadeInicial;
}

void borda(struct Cobra *cobra, struct Comida *comida) {
    screenClear();
    printf(" ");
    for (int i = 0; i < largura; i++) {
        printf("-");
    }
    printf("\n");
    for (int i = 0; i < altura; i++) {
        printf("|");
        for (int j = 0; j < largura; j++) {
            int posicaoCobra = 0;
            struct CaudaCobra *atual = cobra->cabeca;
            while (atual != NULL) {
                if (atual->posicao.x == j && atual->posicao.y == i) {
                    printf(corazul "l" resetarRestante); 
                    posicaoCobra = 1;
                    break;
                }
                atual = atual->ptr;
            }
            if (!posicaoCobra) {
                if (comida->posicao.x == j && comida->posicao.y == i) {
                    printf(corvermelha "O" resetarRestante);
                } else {
                    printf(" ");
                }
            }
        }
        printf("|\n");
    }
    printf(" ");
    for (int i = 0; i < largura; i++) {
        printf("-");
    }
    printf("\n");
}

void atualizarCobra(struct Cobra *cobra) {
    struct CaudaCobra *novaCabeca = (struct CaudaCobra *)malloc(sizeof(struct CaudaCobra));
    novaCabeca->posicao.x = cobra->cabeca->posicao.x + cobra->direcao.x;
    novaCabeca->posicao.y = cobra->cabeca->posicao.y + cobra->direcao.y;
    novaCabeca->ptr = cobra->cabeca;
    cobra->cabeca = novaCabeca;
    if (novaCabeca->posicao.x < 0 || novaCabeca->posicao.x >= largura || novaCabeca->posicao.y < 0 || novaCabeca->posicao.y >= altura) {
        printf(corvermelha "GAME OVER! " resetarRestante);
        printf(coramarelo "SCORE:" resetarRestante " %d\n", cobra->comprimento - 1);
        safeScore(cobra->comprimento - 1);
        printf("\nPontuações passadas:\n");
        carregarScore();
        alocarMemoriaCobra(cobra);  
        exit(0);
    }
}

void fruta(struct Cobra *cobra, struct Comida *comida, int *velocidade) {
    if (cobra->cabeca->posicao.x == comida->posicao.x && cobra->cabeca->posicao.y == comida->posicao.y) {
        cobra->comprimento++;
        comida->posicao.x = rand() % largura;
        comida->posicao.y = rand() % altura;
        if (*velocidade > velocidadePosmaca) {
            *velocidade -= velocidadePosmaca;
        } else {
            *velocidade = velocidadePosmaca;
        }
    } else {
        struct CaudaCobra *corpo = cobra->cabeca;
        while (corpo->ptr->ptr != NULL) {
            corpo = corpo->ptr;
        }
        free(corpo->ptr);  
        corpo->ptr = NULL;
        cobra->cauda = corpo;  
    }
}

void tela() {
    screenClear();
    screenUpdate();
}

void manu() {
    screenClear();
    printf("Bem-vindo ao jogo da cobrinha!\n\n");
    printf("Pressione qualquer botão para começar :)\n");
    printf("Para pausar o jogo pressione 'q'\n");
}

int main() {
    struct Cobra cobra;
    struct Comida comida;
    char teclapre;
    int pontuacao = 0;
    int velocidade;
    int pausa = 0;  
    srand(time(NULL));
    keyboardInit();
    tela();
    while (1) {
        manu();
        getchar();
        inicio(&cobra, &comida, &velocidade);
        while (1) {
            if (keyhit()) {
                teclapre = readch();
                switch (teclapre) {
                case 'w':
                    if (cobra.direcao.y == 0) {
                        cobra.direcao.x = 0;
                        cobra.direcao.y = -1;
                    }
                    break;
                case 's':
                    if (cobra.direcao.y == 0) {
                        cobra.direcao.x = 0;
                        cobra.direcao.y = 1;
                    }
                    break;
                case 'a':
                    if (cobra.direcao.x == 0) {
                        cobra.direcao.x = -1;
                        cobra.direcao.y = 0;
                    }
                    break;
                case 'd':
                    if (cobra.direcao.x == 0) {
                        cobra.direcao.x = 1;
                        cobra.direcao.y = 0;
                    }
                    break;
                case 'q':  
                    if (pausa == 0) {
                        pausa = 1;
                        screenClear();
                        printf("Jogo pausado. Pressione Q para continuar.\n");
                        getchar();  
                    } else {
                        pausa = 0;
                    }
                    break;
                }
            }

            if (pausa == 0) {  
                atualizarCobra(&cobra);
                fruta(&cobra, &comida, &velocidade);
                borda(&cobra, &comida);
                usleep(velocidade);
                pontuacao = cobra.comprimento - 1;
            }
        }
    }
    return 0;
}#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "keyboard.h"
#include "screen.h"

#define largura 60
#define altura 30
#define arquivoScore "score.txt"
#define velocidadeInicial 200000
#define velocidadePosmaca 250000
#define corazul "\x1b[34m"
#define corvermelha "\x1b[31m"
#define coramarelo "\x1b[33m"
#define resetarRestante "\x1b[0m"

struct Ponto {
    int x, y;
};

struct CaudaCobra {
    struct Ponto posicao;
    struct CaudaCobra *ptr;
};

struct Cobra {
    struct CaudaCobra *cabeca;
    struct CaudaCobra *cauda;
    int comprimento;
    struct Ponto direcao;
};

struct Comida {
    struct Ponto posicao;
};

void safeScore(int pontuacao) {
    FILE *arquivo = fopen(arquivoScore, "a");
    if (arquivo == NULL) {
        fprintf(stderr, "erro ao abrir o arquivo: %s\n", arquivoScore);
        return;
    }
    fprintf(arquivo, "%d\n", pontuacao);
    fclose(arquivo);
}

void carregarScore() {
    FILE *arquivo = fopen(arquivoScore, "r");
    if (arquivo == NULL) {
        fprintf(stderr, "erro ao abrir o arquivo: %s\n", arquivoScore);
        return;
    }
    int pontuacao;
    while (fscanf(arquivo, "%d", &pontuacao) != EOF) {
        printf("%d ", pontuacao);
    }
    fclose(arquivo);
}


void alocarMemoriaCobra(struct Cobra *cobra) {
    struct CaudaCobra *corpo = cobra->cabeca;
    while (corpo != NULL) {
        struct CaudaCobra *ptr = corpo->ptr;
        free(corpo);  
        corpo = ptr;
    }
    cobra->cabeca = NULL;
    cobra->cauda = NULL;
}

void inicio(struct Cobra *cobra, struct Comida *comida, int *velocidade) {
    struct CaudaCobra *cabecaCauda = (struct CaudaCobra *)malloc(sizeof(struct CaudaCobra));
    cabecaCauda->posicao.x = largura / 2;
    cabecaCauda->posicao.y = altura / 2;
    cabecaCauda->ptr = NULL;
    cobra->cabeca = cabecaCauda;
    cobra->cauda = cabecaCauda;  
    cobra->comprimento = 1;
    cobra->direcao.x = 1;
    cobra->direcao.y = 0;
    comida->posicao.x = rand() % largura;
    comida->posicao.y = rand() % altura;
    *velocidade = velocidadeInicial;
}

void borda(struct Cobra *cobra, struct Comida *comida) {
    screenClear();
    printf(" ");
    for (int i = 0; i < largura; i++) {
        printf("#");
    }
    printf("\n");
    for (int i = 0; i < altura; i++) {
        printf("#");
        for (int j = 0; j < largura; j++) {
            int posicaoCobra = 0;
            struct CaudaCobra *atual = cobra->cabeca;
            while (atual != NULL) {
                if (atual->posicao.x == j && atual->posicao.y == i) {
                    printf(corazul "y" resetarRestante); 
                    posicaoCobra = 1;
                    break;
                }
                atual = atual->ptr;
            }
            if (!posicaoCobra) {
                if (comida->posicao.x == j && comida->posicao.y == i) {
                    printf(corvermelha "O" resetarRestante);
                } else {
                    printf(" ");
                }
            }
        }
        printf("#\n");
    }
    printf(" ");
    for (int i = 0; i < largura; i++) {
        printf("#");
    }
    printf("\n");
}

void atualizarCobra(struct Cobra *cobra) {
    struct CaudaCobra *novaCabeca = (struct CaudaCobra *)malloc(sizeof(struct CaudaCobra));
    novaCabeca->posicao.x = cobra->cabeca->posicao.x + cobra->direcao.x;
    novaCabeca->posicao.y = cobra->cabeca->posicao.y + cobra->direcao.y;
    novaCabeca->ptr = cobra->cabeca;
    cobra->cabeca = novaCabeca;
    if (novaCabeca->posicao.x < 0 || novaCabeca->posicao.x >= largura || novaCabeca->posicao.y < 0 || novaCabeca->posicao.y >= altura) {
        printf(corvermelha "GAME OVER! " resetarRestante);
        printf(coramarelo "SCORE:" resetarRestante " %d\n", cobra->comprimento - 1);
        safeScore(cobra->comprimento - 1);
        printf("\nPontuações passadas:\n");
        carregarScore();
        alocarMemoriaCobra(cobra);  
        exit(0);
    }
}

void fruta(struct Cobra *cobra, struct Comida *comida, int *velocidade) {
    if (cobra->cabeca->posicao.x == comida->posicao.x && cobra->cabeca->posicao.y == comida->posicao.y) {
        cobra->comprimento++;
        comida->posicao.x = rand() % largura;
        comida->posicao.y = rand() % altura;
        if (*velocidade > velocidadePosmaca) {
            *velocidade -= velocidadePosmaca;
        } else {
            *velocidade = velocidadePosmaca;
        }
    } else {
        struct CaudaCobra *corpo = cobra->cabeca;
        while (corpo->ptr->ptr != NULL) {
            corpo = corpo->ptr;
        }
        free(corpo->ptr);  
        corpo->ptr = NULL;
        cobra->cauda = corpo;  
    }
}

void tela() {
    screenClear();
    screenUpdate();
}

void manu() {
    screenClear();
    printf("Bem-vindo ao jogo da cobrinha!\n\n");
    printf("Pressione qualquer botão para começar :)\n");
    printf("Para pausar o jogo pressione 'q'\n");
}

int main() {
    struct Cobra cobra;
    struct Comida comida;
    char teclapre;
    int pontuacao = 0;
    int velocidade;
    int pausa = 0;  
    srand(time(NULL));
    keyboardInit();
    tela();
    while (1) {
        manu();
        getchar();
        inicio(&cobra, &comida, &velocidade);
        while (1) {
            if (keyhit()) {
                teclapre = readch();
                switch (teclapre) {
                case 'w':
                    if (cobra.direcao.y == 0) {
                        cobra.direcao.x = 0;
                        cobra.direcao.y = -1;
                    }
                    break;
                case 's':
                    if (cobra.direcao.y == 0) {
                        cobra.direcao.x = 0;
                        cobra.direcao.y = 1;
                    }
                    break;
                case 'a':
                    if (cobra.direcao.x == 0) {
                        cobra.direcao.x = -1;
                        cobra.direcao.y = 0;
                    }
                    break;
                case 'd':
                    if (cobra.direcao.x == 0) {
                        cobra.direcao.x = 1;
                        cobra.direcao.y = 0;
                    }
                    break;
                case 'q':  
                    if (pausa == 0) {
                        pausa = 1;
                        screenClear();
                        printf("Jogo pausado. Pressione Q para continuar.\n");
                        getchar();  
                    } else {
                        pausa = 0;
                    }
                    break;
                }
            }

            if (pausa == 0) {  
                atualizarCobra(&cobra);
                fruta(&cobra, &comida, &velocidade);
                borda(&cobra, &comida);
                usleep(velocidade);
                pontuacao = cobra.comprimento - 1;
            }
        }
    }
    return 0;
}
