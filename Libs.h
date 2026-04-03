//
// Created by Soarezz on 30/03/26.
//

#ifndef TP1_ESTRUTURA_DE_DADOS_LIBS_H
#define TP1_ESTRUTURA_DE_DADOS_LIBS_H

typedef struct {
    // textos
    char sigla_tribunal[10];
    char procedimento[100];
    char ramo_justica[50];
    char sigla_grau[10];
    char uf_oj[5];
    char municipio_oj[100];
    char id_ultimo_oj[50];
    char nome[150];
    char mesano_cnm1[20];
    char mesano_sent[20];


    int casos_novos_2026;
    int julgados_2026;
    int prim_sent2026;
    int suspensos_2026;
    int dessobrestados_2026;

    // Resultados das metas aqui
    float cumprimento_meta1;

    // Meta 2
    int distm2_a;
    int julgm2_a;
    int suspm2_a;
    float cumprimento_meta2a;
    int distm2_ant;
    int julgm2_ant;
    int suspm2_ant;
    int desom2_ant;
    float cumprimento_meta2ant;

    // Meta 4
    int distm4_a;
    int julgm4_a;
    int suspm4_a;
    float cumprimento_meta4a;
    int distm4_b;
    int julgm4_b;
    int suspm4_b;
    float cumprimento_meta4b;
} Processo;

typedef struct {
    Processo *Dados;   // Ponteiro q aponta p processos ( struct de cima )
    int Tamanho;
    int Capacidade;
} Lista;


Lista* criar_lista(int cap);

void destruir_lista(Lista *L);

void adicionar_processo(Lista *L, Processo P);

void concatenar_arquivos(Lista *L);

void gerar_resumo(Lista *L);

void filtrar_municipio(Lista *L, char *nome_busca);


#endif //TP1_ESTRUTURA_DE_DADOS_LIBS_H
