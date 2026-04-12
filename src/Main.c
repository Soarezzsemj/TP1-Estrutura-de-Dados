#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "Libs.h"

static void normalizar_uf(char *uf) {
    if (!uf) return;

    int escrita = 0;
    for (int leitura = 0; uf[leitura] != '\0'; leitura++) {
        if (uf[leitura] == ' ') continue;
        uf[escrita++] = (char)toupper((unsigned char)uf[leitura]);
    }
    uf[escrita] = '\0';
}

static int uf_valida(const char *uf) {
    static const char *ufs_validas[] = {
        "AC", "AL", "AP", "AM", "BA", "CE", "DF", "ES", "GO",
        "MA", "MT", "MS", "MG", "PA", "PB", "PR", "PE", "PI",
        "RJ", "RN", "RS", "RO", "RR", "SC", "SP", "SE", "TO"
    };

    if (!uf ||
        strlen(uf) != 2 ||
        !isalpha((unsigned char)uf[0]) ||
        !isalpha((unsigned char)uf[1])) {
        return 0;
    }

    int total_ufs = (int)(sizeof(ufs_validas) / sizeof(ufs_validas[0]));
    for (int i = 0; i < total_ufs; i++) {
        if (strcmp(uf, ufs_validas[i]) == 0) return 1;
    }
    return 0;
}

static int resolver_caminho_arquivo(char *destino, size_t tamanho_destino, const char *arquivo) {
    const char *pastas[] = { "dados/testes", "testes", "." };

    for (int i = 0; i < 3; i++) {
        int n = snprintf(destino, tamanho_destino, "%s/%s", pastas[i], arquivo);
        if (n < 0 || n >= (int)tamanho_destino) continue;

        FILE *f = fopen(destino, "r");
        if (f) {
            fclose(f);
            return 1;
        }
    }
    return 0;
}

int main(void) {
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

    Lista *L = criar_lista(100000);
    if (!L) {
        fprintf(stderr, "Erro fatal: nao foi possivel inicializar a lista de processos.\n");
        return 1;
    }

    printf("Carregando %d arquivos...\n", n_arquivos);
    for (int i = 0; i < n_arquivos; i++) {
        char caminho[256];
        if (!resolver_caminho_arquivo(caminho, sizeof caminho, arquivos[i])) {
            snprintf(caminho, sizeof caminho, "%s", arquivos[i]);
        }

        printf("  [%2d/%d] %s\n", i + 1, n_arquivos, arquivos[i]);
        carregar_arquivo(L, caminho);
    }
    printf("Total de registros carregados: %d\n\n", L->Tamanho);

    printf("1. Concatenando todos os dados em cvs.csv...\n");
    concatenar_arquivos(L);

    printf("2. Gerando resumo.csv (5 metas por tribunal)...\n");
    gerar_resumo(L);

    printf("\n4. Digite o estado (UF) para gerar resumo filtrado (ex: SP, RJ, AC): ");
    char estado[10];
    if (fgets(estado, sizeof estado, stdin)) {
        estado[strcspn(estado, "\r\n")] = '\0';
        normalizar_uf(estado);
        if (estado[0] == '\0') {
            printf("   UF vazia. Resumo por estado ignorado.\n");
        } else if (!uf_valida(estado)) {
            printf("   UF invalida (%s). Informe uma UF brasileira valida (ex: SP, RJ, AC).\n", estado);
        } else {
            gerar_resumo_por_estado(L, estado);
        }
    } else {
        fprintf(stderr, "Erro ao ler a UF informada.\n");
    }

    printf("5. Digite o municipio para filtrar: ");
    char busca[100];
    if (!fgets(busca, sizeof busca, stdin)) {
        fprintf(stderr, "Erro ao ler municipio.\n");
        destruir_lista(L);
        return 1;
    }
    busca[strcspn(busca, "\r\n")] = '\0';
    if (busca[0] == '\0') {
        printf("   Municipio vazio. Filtro por municipio ignorado.\n");
    } else {
        filtrar_municipio(L, busca);
    }

    destruir_lista(L);
    printf("\nConcluido.\n");
    return 0;
}
