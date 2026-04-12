#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif
#include "Libs.h"

/* ===========================================
   FUNÇÕES AUXILIARES INTERNAS
   =========================================== */

static const char *PASTA_SAIDA = "resultados";

/* Garante que a pasta de saída exista */
static int garantir_pasta_saida(void) {
#ifdef _WIN32
    if (_mkdir(PASTA_SAIDA) == 0 || errno == EEXIST) return 1;
#else
    if (mkdir(PASTA_SAIDA, 0777) == 0 || errno == EEXIST) return 1;
#endif
    fprintf(stderr, "Erro ao criar pasta de saida: %s\n", PASTA_SAIDA);
    return 0;
}

/* Monta caminho completo dentro da pasta de saída */
static int montar_caminho_saida(char *dest, size_t dest_sz, const char *nome_arquivo) {
    if (!dest || dest_sz == 0 || !nome_arquivo || nome_arquivo[0] == '\0') return 0;
    int n = snprintf(dest, dest_sz, "%s/%s", PASTA_SAIDA, nome_arquivo);
    return n >= 0 && n < (int)dest_sz;
}

/* UF valida: exatamente 2 letras e pertencente aos estados do Brasil */
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

/* Versão própria da função strsep (que não existe no Windows) */
static char *meu_strsep(char **sp, const char *delim) {
    char *s = *sp;
    if (!s) return NULL;
    char *p = s;
    while (*p && !strchr(delim, *p)) p++;
    if (*p) { *p = '\0'; *sp = p + 1; }
    else      *sp = NULL;
    return s;
}

/* Remove caracteres de quebra de linha e espaços do final da string */
static void trim_eol(char *s) {
    int n = (int)strlen(s);
    while (n > 0 && (s[n-1]=='\r' || s[n-1]=='\n' || s[n-1]==' ' || s[n-1]=='"'))
        s[--n] = '\0';
    /* Remove aspas do início */
    if (s[0] == '"') {
        int j = 0;
        while (s[j+1] != '\0') {
            s[j] = s[j+1];
            j++;
        }
        s[j] = '\0';
    }
}

/* Compara duas strings ignorando diferenças entre maiúsculas e minúsculas */
static int cmp_ci(const char *a, const char *b) {
    while (*a && *b)
        if (tolower((unsigned char)*a++) != tolower((unsigned char)*b++)) return 0;
    return *a == '\0' && *b == '\0';
}

/* Gera um nome de arquivo seguro a partir do nome do municipio */
static void montar_nome_arquivo_municipio(char *dest, size_t dest_sz, const char *busca) {
    size_t j = 0;
    if (!dest || dest_sz == 0) return;

    for (size_t i = 0; busca && busca[i] != '\0' && j + 1 < dest_sz; i++) {
        unsigned char c = (unsigned char)busca[i];
        if (isalnum(c) || c == '-' || c == '_') {
            dest[j++] = (char)c;
        } else if (c == ' ') {
            dest[j++] = '_';
        }
    }

    if (j == 0) {
        snprintf(dest, dest_sz, "municipio_filtrado");
    } else {
        dest[j] = '\0';
    }
}

/* Detecta automaticamente se o separador do CSV é vírgula ou ponto-e-vírgula */
static char detectar_sep(const char *linha) {
    return strchr(linha, ';') ? ';' : ',';
}

/* Estrutura auxiliar para agrupar dados por tribunal */
typedef struct {
    char sigla[12];  /* Sigla do tribunal (ex: TRE-AC) */
    long jul1,  nov1,  susp1,  des1;         /* Para Meta1 */
    long jul2a, dist2a, susp2a;              /* Para Meta2A */
    long jul2an, dist2an, susp2an, des2an;   /* Para Meta2Ant */
    long jul4a, dist4a, susp4a;              /* Para Meta4A */
    long jul4b, dist4b, susp4b;              /* Para Meta4B */
} RT;

/* Preenche o campo correto do processo conforme a coluna do CSV */
static void preencher_processo_por_coluna(Processo *p, int col, const char *tok) {
    switch (col) {
        case  0: strncpy(p->sigla_tribunal,   tok, 11); break;
        case  1: strncpy(p->procedimento,     tok, 99); break;
        case  2: strncpy(p->ramo_justica,     tok, 59); break;
        case  3: strncpy(p->sigla_grau,       tok,  9); break;
        case  4: strncpy(p->uf_oj,            tok,  4); break;
        case  5: strncpy(p->municipio_oj,     tok, 99); break;
        case  6: strncpy(p->id_ultimo_oj,     tok, 59); break;
        case  7: strncpy(p->nome,             tok,199); break;
        case  8: strncpy(p->mesano_cnm1,      tok, 19); break;
        case  9: strncpy(p->mesano_sent,      tok, 19); break;
        case 10: p->casos_novos_2026     = atoi(tok);         break;
        case 11: p->julgados_2026        = atoi(tok);         break;
        case 12: p->prim_sent2026        = atoi(tok);         break;
        case 13: p->suspensos_2026       = atoi(tok);         break;
        case 14: p->dessobrestados_2026  = atoi(tok);         break;
        case 15: p->cumprimento_meta1    = (float)atof(tok);  break;
        case 16: p->distm2_a             = atoi(tok);         break;
        case 17: p->julgm2_a             = atoi(tok);         break;
        case 18: p->suspm2_a             = atoi(tok);         break;
        case 19: p->cumprimento_meta2a   = (float)atof(tok);  break;
        case 20: p->distm2_ant           = atoi(tok);         break;
        case 21: p->julgm2_ant           = atoi(tok);         break;
        case 22: p->suspm2_ant           = atoi(tok);         break;
        case 23: p->desom2_ant           = atoi(tok);         break;
        case 24: p->cumprimento_meta2ant = (float)atof(tok);  break;
        case 25: p->distm4_a             = atoi(tok);         break;
        case 26: p->julgm4_a             = atoi(tok);         break;
        case 27: p->suspm4_a             = atoi(tok);         break;
        case 28: p->cumprimento_meta4a   = (float)atof(tok);  break;
        case 29: p->distm4_b             = atoi(tok);         break;
        case 30: p->julgm4_b             = atoi(tok);         break;
        case 31: p->suspm4_b             = atoi(tok);         break;
        case 32: p->cumprimento_meta4b   = (float)atof(tok);  break;
        default: break;
    }
}

/* Retorna (ou cria) o índice do tribunal no vetor auxiliar */
static int obter_indice_tribunal(RT trib[], int *n, const char *sigla) {
    for (int j = 0; j < *n; j++) {
        if (strcmp(trib[j].sigla, sigla) == 0) return j;
    }

    if (*n >= 200) return -1;

    int idx = (*n)++;
    memset(&trib[idx], 0, sizeof(RT));
    strncpy(trib[idx].sigla, sigla, 11);
    return idx;
}

/* Soma os valores de um processo nas estruturas de meta do tribunal */
static void acumular_metas(RT *t, const Processo *p) {
    t->jul1    += p->julgados_2026;
    t->nov1    += p->casos_novos_2026;
    t->susp1   += p->suspensos_2026;
    t->des1    += p->dessobrestados_2026;
    t->jul2a   += p->julgm2_a;
    t->dist2a  += p->distm2_a;
    t->susp2a  += p->suspm2_a;
    t->jul2an  += p->julgm2_ant;
    t->dist2an += p->distm2_ant;
    t->susp2an += p->suspm2_ant;
    t->des2an  += p->desom2_ant;
    t->jul4a   += p->julgm4_a;
    t->dist4a  += p->distm4_a;
    t->susp4a  += p->suspm4_a;
    t->jul4b   += p->julgm4_b;
    t->dist4b  += p->distm4_b;
    t->susp4b  += p->suspm4_b;
}

/* Calcula as 5 metas do tribunal */
static void calcular_metas(const RT *t, double *m1, double *m2a, double *m2an, double *m4a, double *m4b) {
    long d1   = t->nov1  + t->des1  - t->susp1;
    long d2a  = t->dist2a  - t->susp2a;
    long d2an = t->dist2an - t->susp2an - t->des2an;
    long d4a  = t->dist4a  - t->susp4a;
    long d4b  = t->dist4b  - t->susp4b;

    *m1 = *m2a = *m2an = *m4a = *m4b = 0.0;
    if (d1   != 0) *m1   = (double)t->jul1  / d1   * 100.0;
    if (d2a  != 0) *m2a  = (double)t->jul2a / d2a  * (1000.0 / 7.0);
    if (d2an != 0) *m2an = (double)t->jul2an/ d2an * 100.0;
    if (d4a  != 0) *m4a  = (double)t->jul4a / d4a  * 100.0;
    if (d4b  != 0) *m4b  = (double)t->jul4b / d4b  * 100.0;
}

/* Escreve cabeçalho padrão do resumo */
static void escrever_cabecalho_resumo(FILE *f) {
    fprintf(f, "sigla_tribunal;total_julgados_2026;Meta1;Meta2A;Meta2Ant;Meta4A;Meta4B\n");
}

/* Escreve uma linha de resumo (um tribunal) */
static void escrever_linha_resumo(FILE *f, const RT *t) {
    double m1, m2a, m2an, m4a, m4b;
    calcular_metas(t, &m1, &m2a, &m2an, &m4a, &m4b);
    fprintf(f, "%s;%ld;%.2f%%;%.2f%%;%.2f%%;%.2f%%;%.2f%%\n",
            t->sigla, t->jul1, m1, m2a, m2an, m4a, m4b);
}

/* ===========================================
   ESTRUTURA DE DADOS: LISTA DINÂMICA
   =========================================== */

Lista *criar_lista(int cap) {
    if (cap <= 0) {
        fprintf(stderr, "Erro: capacidade inicial invalida (%d).\n", cap);
        return NULL;
    }

    Lista *L = (Lista*)malloc(sizeof(Lista));
    if (!L) {
        fprintf(stderr, "Erro: falha ao alocar estrutura da lista.\n");
        return NULL;
    }

    L->Dados     = (Processo*)malloc(sizeof(Processo) * cap);
    if (!L->Dados) {
        fprintf(stderr, "Erro: falha ao alocar vetor de processos.\n");
        free(L);
        return NULL;
    }

    L->Tamanho   = 0;
    L->Capacidade = cap;
    return L;
}

void destruir_lista(Lista *L) {
    if (!L) return;
    free(L->Dados);
    free(L);
}

void adicionar_processo(Lista *L, Processo P) {
    if (!L || !L->Dados) {
        fprintf(stderr, "Erro: lista invalida ao adicionar processo.\n");
        return;
    }

    if (L->Tamanho == L->Capacidade) {
        L->Capacidade *= 2;  /* Dobra a capacidade quando cheia */
        Processo *novo = (Processo*)realloc(L->Dados, sizeof(Processo) * L->Capacidade);
        if (!novo) {
            fprintf(stderr, "Erro: falha ao expandir a lista de processos.\n");
            return;
        }
        L->Dados = novo;
    }
    L->Dados[L->Tamanho++] = P;
}

/* ===========================================
   CARREGAR DADOS DE UM ARQUIVO CSV
   Lê todas as 33 colunas de dados do arquivo
   =========================================== */

void carregar_arquivo(Lista *L, const char *nome) {
    if (!L || !L->Dados || !nome || nome[0] == '\0') {
        fprintf(stderr, "Erro: parametros invalidos em carregar_arquivo.\n");
        return;
    }

    FILE *f = fopen(nome, "r");
    if (!f) { fprintf(stderr, "Aviso: arquivo nao encontrado: %s\n", nome); return; }

    char linha[4096];
    /* Lê a primeira linha (cabeçalho) apenas para detectar o separador */
    if (!fgets(linha, sizeof linha, f)) { fclose(f); return; }
    char sep = detectar_sep(linha);
    char sep_str[3] = { sep, '\n', '\0' };

    /* Processa cada linha do arquivo */
    while (fgets(linha, sizeof linha, f)) {
        Processo p;
        memset(&p, 0, sizeof p);  /* Zera todos os campos */
        char *ptr = linha;
        char *tok;
        int col = 0;

        /* Separa a linha em colunas usando o separador detectado */
        while ((tok = meu_strsep(&ptr, sep_str)) != NULL) {
            trim_eol(tok);  /* Remove quebras de linha */
            preencher_processo_por_coluna(&p, col, tok);
            col++;
        }
        if (col > 5) adicionar_processo(L, p); /* Ignora linhas vazias ou incompletas */
    }
    fclose(f);
}

/* Macro auxiliar para escrever uma linha completa de dados no formato CSV */
#define ESCREVER_LINHA(fp, p)                                           \
    fprintf((fp),                                                        \
        "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,"                                \
        "%d,%d,%d,%d,%d,%.4f,"                                          \
        "%d,%d,%d,%.4f,"                                                \
        "%d,%d,%d,%d,%.4f,"                                             \
        "%d,%d,%d,%.4f,"                                                \
        "%d,%d,%d,%.4f\n",                                              \
        (p)->sigla_tribunal,(p)->procedimento,(p)->ramo_justica,        \
        (p)->sigla_grau,(p)->uf_oj,(p)->municipio_oj,                   \
        (p)->id_ultimo_oj,(p)->nome,(p)->mesano_cnm1,(p)->mesano_sent,  \
        (p)->casos_novos_2026,(p)->julgados_2026,(p)->prim_sent2026,    \
        (p)->suspensos_2026,(p)->dessobrestados_2026,                   \
        (p)->cumprimento_meta1,                                         \
        (p)->distm2_a,(p)->julgm2_a,(p)->suspm2_a,                     \
        (p)->cumprimento_meta2a,                                        \
        (p)->distm2_ant,(p)->julgm2_ant,(p)->suspm2_ant,               \
        (p)->desom2_ant,(p)->cumprimento_meta2ant,                      \
        (p)->distm4_a,(p)->julgm4_a,(p)->suspm4_a,                     \
        (p)->cumprimento_meta4a,                                        \
        (p)->distm4_b,(p)->julgm4_b,(p)->suspm4_b,                     \
        (p)->cumprimento_meta4b)

/* Cabeçalho do arquivo CSV com os nomes das colunas */
#define CABECALHO_CSV \
    "sigla_tribunal,procedimento,ramo_justica,sigla_grau,uf_oj,"        \
    "municipio_oj,id_ultimo_oj,nome,mesano_cnm1,mesano_sent,"           \
    "casos_novos_2026,julgados_2026,prim_sent2026,suspensos_2026,"      \
    "dessobrestados_2026,cumprimento_meta1,distm2_a,julgm2_a,suspm2_a,"\
    "cumprimento_meta2a,distm2_ant,julgm2_ant,suspm2_ant,desom2_ant,"  \
    "cumprimento_meta2ant,distm4_a,julgm4_a,suspm4_a,cumprimento_meta4a,"\
    "distm4_b,julgm4_b,suspm4_b,cumprimento_meta4b\n"

/* ===========================================
   CONCATENAR TODOS OS DADOS EM UM ÚNICO CSV
   Cria o arquivo cvs.csv com todos os registros
   =========================================== */

void concatenar_arquivos(Lista *L) {
    if (!L || !L->Dados) {
        fprintf(stderr, "Erro: lista invalida para concatenar arquivos.\n");
        return;
    }

    if (!garantir_pasta_saida()) return;

    char caminho_saida[260];
    if (!montar_caminho_saida(caminho_saida, sizeof caminho_saida, "cvs.csv")) {
        fprintf(stderr, "Erro ao montar caminho de saida para cvs.csv\n");
        return;
    }

    FILE *f = fopen(caminho_saida, "w");
    if (!f) { fprintf(stderr, "Erro ao criar %s\n", caminho_saida); return; }

    fprintf(f, CABECALHO_CSV);  /* Escreve o cabeçalho */
    for (int i = 0; i < L->Tamanho; i++)
        ESCREVER_LINHA(f, &L->Dados[i]);  /* Escreve cada registro */

    fclose(f);
    printf("   %s gerado com %d registros.\n", caminho_saida, L->Tamanho);
}

/* ===========================================
   GERAR RESUMO POR TRIBUNAL
   Cria resumo.csv com estatísticas de 5 metas para cada tribunal

   FÓRMULAS DAS METAS (conforme enunciado):
   Meta1    = Σjulgados_2026 / (Σcasos_novos + Σdessobrestados - Σsuspensos) × 100%
   Meta2A   = Σjulgm2_a / (Σdistm2_a - Σsuspm2_a) × (1000/7)
   Meta2Ant = Σjulgm2_ant / (Σdistm2_ant - Σsuspm2_ant - Σdesom2_ant) × 100%
   Meta4A   = Σjulgm4_a / (Σdistm4_a - Σsuspm4_a) × 100%
   Meta4B   = Σjulgm4_b / (Σdistm4_b - Σsuspm4_b) × 100%
   =========================================== */

void gerar_resumo(Lista *L) {
    if (!L || !L->Dados) {
        fprintf(stderr, "Erro: lista invalida para gerar resumo.\n");
        return;
    }

    RT trib[200];   /* Suporta até 200 tribunais diferentes */
    int n = 0;      /* Número de tribunais encontrados */

    /* Agrupa os dados por tribunal */
    for (int i = 0; i < L->Tamanho; i++) {
        Processo *p = &L->Dados[i];
        int idx = obter_indice_tribunal(trib, &n, p->sigla_tribunal);
        if (idx < 0) continue;  /* Limite de tribunais atingido */
        acumular_metas(&trib[idx], p);
    }

    if (!garantir_pasta_saida()) return;

    /* Cria o arquivo resumo.csv */
    char caminho_saida[260];
    if (!montar_caminho_saida(caminho_saida, sizeof caminho_saida, "resumo.csv")) {
        fprintf(stderr, "Erro ao montar caminho de saida para resumo.csv\n");
        return;
    }

    FILE *f = fopen(caminho_saida, "w");
    if (!f) { fprintf(stderr, "Erro ao criar %s\n", caminho_saida); return; }

    /* Escreve o cabeçalho */
    escrever_cabecalho_resumo(f);

    /* Calcula e escreve as metas para cada tribunal */
    for (int i = 0; i < n; i++) {
        escrever_linha_resumo(f, &trib[i]);
    }
    fclose(f);
    printf("   %s gerado com %d tribunais.\n", caminho_saida, n);
}

/* Função para gerar resumo filtrado por estado (UF) */
void gerar_resumo_por_estado(Lista *L, const char *uf) {
    if (!L || !L->Dados || !uf_valida(uf)) {
        fprintf(stderr, "Erro: UF invalida. Informe uma UF brasileira valida (ex: SP).\n");
        return;
    }

    RT trib[200];
    int n = 0;

    /* Agrupa dados por tribunal, apenas do estado especificado */
    for (int i = 0; i < L->Tamanho; i++) {
        Processo *p = &L->Dados[i];

        /* Verifica se o registro pertence ao estado solicitado (case-insensitive) */
        if (tolower((unsigned char)p->uf_oj[0]) != tolower((unsigned char)uf[0]) ||
            tolower((unsigned char)p->uf_oj[1]) != tolower((unsigned char)uf[1])) {
            continue;
        }

        int idx = obter_indice_tribunal(trib, &n, p->sigla_tribunal);
        if (idx < 0) continue;
        acumular_metas(&trib[idx], p);
    }

    /* Se não encontrou dados para este estado */
    if (n == 0) {
        printf("   Nenhum tribunal encontrado para o estado %s.\n", uf);
        return;
    }

    /* Cria o arquivo resumo_{UF}.csv */
    char nome_arq[100];
    snprintf(nome_arq, sizeof(nome_arq), "resumo_%s.csv", uf);

    if (!garantir_pasta_saida()) return;

    char caminho_saida[260];
    if (!montar_caminho_saida(caminho_saida, sizeof caminho_saida, nome_arq)) {
        fprintf(stderr, "Erro ao montar caminho de saida para %s\n", nome_arq);
        return;
    }

    FILE *f = fopen(caminho_saida, "w");
    if (!f) { fprintf(stderr, "Erro ao criar %s\n", caminho_saida); return; }

    /* Escreve o cabeçalho */
    escrever_cabecalho_resumo(f);

    /* Calcula e escreve as metas para cada tribunal do estado */
    for (int i = 0; i < n; i++) {
        escrever_linha_resumo(f, &trib[i]);
    }
    fclose(f);
    printf("   %s gerado com %d tribunal(is) do estado %s.\n", caminho_saida, n, uf);
}

/* ===========================================
   FILTRAR DADOS POR MUNICÍPIO
   Cria um arquivo CSV específico com registros de um município
   =========================================== */

void filtrar_municipio(Lista *L, const char *busca) {
    if (!L || !L->Dados || !busca || busca[0] == '\0') {
        fprintf(stderr, "Erro: parametros invalidos para filtrar municipio.\n");
        return;
    }

    char municipio_sanitizado[180];
    montar_nome_arquivo_municipio(municipio_sanitizado, sizeof municipio_sanitizado, busca);

    char nome_arq[200];
    int n = snprintf(nome_arq, sizeof nome_arq, "%s.csv", municipio_sanitizado);
    if (n < 0 || n >= (int)sizeof nome_arq) {
        fprintf(stderr, "Erro: nome de arquivo muito grande.\n");
        return;
    }

    if (!garantir_pasta_saida()) return;

    char caminho_saida[260];
    if (!montar_caminho_saida(caminho_saida, sizeof caminho_saida, nome_arq)) {
        fprintf(stderr, "Erro ao montar caminho de saida para %s\n", nome_arq);
        return;
    }

    FILE *f = fopen(caminho_saida, "w");
    if (!f) { fprintf(stderr, "Erro ao criar %s\n", caminho_saida); return; }

    fprintf(f, CABECALHO_CSV);  /* Escreve o cabeçalho */

    int encontrados = 0;
    for (int i = 0; i < L->Tamanho; i++) {
        Processo *p = &L->Dados[i];
        if (cmp_ci(p->municipio_oj, busca)) {  /* Compara ignorando maiúsculas/minúsculas */
            ESCREVER_LINHA(f, p);  /* Escreve o registro no arquivo */
            encontrados++;
        }
    }
    fclose(f);
    printf("   %d registro(s) encontrado(s) -> %s\n", encontrados, caminho_saida);
}