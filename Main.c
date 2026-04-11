#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "Libs.h"

int main(void) {

    /* Lista com os nomes dos 27 arquivos CSV dos tribunais eleitorais */
    const char *arquivos[] = {
        "teste_TRE-AC.csv", "teste_TRE-AL.csv", "teste_TRE-AM.csv",
        "teste_TRE-AP.csv", "teste_TRE-BA.csv", "teste_TRE-CE.csv",
        "teste_TRE-DF.csv", "teste_TRE-ES.csv", "teste_TRE-GO.csv",
        "teste_TRE-MA.csv", "teste_TRE-MG.csv", "teste_TRE-MS.csv",
        "teste_TRE-MT.csv", "teste_TRE-PA.csv", "teste_TRE-PB.csv",
        "teste_TRE-PE.csv", "teste_TRE-PI.csv", "teste_TRE-PR.csv",
        "teste_TRE-RJ.csv", "teste_TRE-RN.csv", "teste_TRE-RO.csv",
        "teste_TRE-RR.csv", "teste_TRE-RS.csv", "teste_TRE-SC.csv",
        "teste_TRE-SE.csv", "teste_TRE-SP.csv", "teste_TRE-TO.csv"
    };
    int n_arquivos = (int)(sizeof(arquivos) / sizeof(arquivos[0]));

    /* Cria uma lista grande o suficiente para armazenar todos os dados */
    Lista *L = criar_lista(100000);

    /* Passo 1: Carrega todos os arquivos CSV na memória */
    printf("Carregando %d arquivos...\n", n_arquivos);
    for (int i = 0; i < n_arquivos; i++) {
        char caminho[256];
        FILE *teste = fopen(arquivos[i], "r");
        if (teste) {
            fclose(teste);
            snprintf(caminho, sizeof(caminho), "%s", arquivos[i]);
        } else {
            snprintf(caminho, sizeof(caminho), "../%s", arquivos[i]);
        }
        printf("  [%2d/%d] %s\n", i+1, n_arquivos, arquivos[i]);
        carregar_arquivo(L, caminho);
    }
    printf("Total de registros carregados: %d\n\n", L->Tamanho);

    /* Passo 2: Junta todos os dados em um único arquivo CSV */
    printf("1. Concatenando todos os dados em cvs.csv...\n");
    concatenar_arquivos(L);

    /* Passo 3: Cria um resumo com estatísticas por tribunal */
    printf("2. Gerando resumo.csv (5 metas por tribunal)...\n");
    gerar_resumo(L);

    /* Passo 4: Permite gerar resumo filtrado por estado */
    printf("\n4. Digite o estado (UF) para gerar resumo filtrado (ex: SP, RJ, AC): ");
    char estado[10];
    if (fgets(estado, sizeof estado, stdin)) {
        estado[strcspn(estado, "\r\n")] = '\0';
        /* Remove espaços em branco */
        for (int i = 0; i < (int)strlen(estado); i++) {
            if (estado[i] == ' ') {
                int j = i;
                while (estado[j] != '\0') {
                    estado[j] = estado[j+1];
                    j++;
                }
                i--;
            }
        }
        /* Converte para maiúscula */
        for (int i = 0; estado[i] != '\0'; i++) {
            estado[i] = (char)toupper((unsigned char)estado[i]);
        }
        if (estado[0] != '\0') {
            gerar_resumo_por_estado(L, estado);
        }
    }

    /* Passo 5: Permite filtrar os dados por município */
    printf("5. Digite o municipio para filtrar: ");
    char busca[100];
    if (!fgets(busca, sizeof busca, stdin)) {
        fprintf(stderr, "Erro ao ler municipio.\n");
        destruir_lista(L);
        return 1;
    }
    busca[strcspn(busca, "\r\n")] = '\0';
    filtrar_municipio(L, busca);

    destruir_lista(L);
    printf("\nConcluido.\n");
    return 0;
}