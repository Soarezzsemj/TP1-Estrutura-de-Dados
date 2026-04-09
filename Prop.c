#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "Libs.h"

/* ===========================================
   FUNÇÕES AUXILIARES INTERNAS
   =========================================== */

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
    while (n > 0 && (s[n-1]=='\r' || s[n-1]=='\n' || s[n-1]==' '))
        s[--n] = '\0';
}

/* Compara duas strings ignorando diferenças entre maiúsculas e minúsculas */
static int cmp_ci(const char *a, const char *b) {
    while (*a && *b)
        if (tolower((unsigned char)*a++) != tolower((unsigned char)*b++)) return 0;
    return *a == '\0' && *b == '\0';
}

/* Detecta automaticamente se o separador do CSV é vírgula ou ponto-e-vírgula */
static char detectar_sep(const char *linha) {
    return strchr(linha, ';') ? ';' : ',';
}

/* ===========================================
   ESTRUTURA DE DADOS: LISTA DINÂMICA
   =========================================== */

Lista *criar_lista(int cap) {
    Lista *L = (Lista*)malloc(sizeof(Lista));
    L->Dados     = (Processo*)malloc(sizeof(Processo) * cap);
    L->Tamanho   = 0;
    L->Capacidade = cap;
    return L;
}

void destruir_lista(Lista *L) {
    free(L->Dados);
    free(L);
}

void adicionar_processo(Lista *L, Processo P) {
    if (L->Tamanho == L->Capacidade) {
        L->Capacidade *= 2;  /* Dobra a capacidade quando cheia */
        L->Dados = (Processo*)realloc(L->Dados, sizeof(Processo) * L->Capacidade);
    }
    L->Dados[L->Tamanho++] = P;
}

/* ===========================================
   CARREGAR DADOS DE UM ARQUIVO CSV
   Lê todas as 33 colunas de dados do arquivo
   =========================================== */

void carregar_arquivo(Lista *L, const char *nome) {
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
            switch (col) {
                case  0: strncpy(p.sigla_tribunal,   tok, 11); break;
                case  1: strncpy(p.procedimento,     tok, 99); break;
                case  2: strncpy(p.ramo_justica,     tok, 59); break;
                case  3: strncpy(p.sigla_grau,       tok,  9); break;
                case  4: strncpy(p.uf_oj,            tok,  4); break;
                case  5: strncpy(p.municipio_oj,     tok, 99); break;
                case  6: strncpy(p.id_ultimo_oj,     tok, 59); break;
                case  7: strncpy(p.nome,             tok,199); break;
                case  8: strncpy(p.mesano_cnm1,      tok, 19); break;
                case  9: strncpy(p.mesano_sent,      tok, 19); break;
                case 10: p.casos_novos_2026     = atoi(tok);         break;
                case 11: p.julgados_2026        = atoi(tok);         break;
                case 12: p.prim_sent2026        = atoi(tok);         break;
                case 13: p.suspensos_2026       = atoi(tok);         break;
                case 14: p.dessobrestados_2026  = atoi(tok);         break;
                case 15: p.cumprimento_meta1    = (float)atof(tok);  break;
                case 16: p.distm2_a             = atoi(tok);         break;
                case 17: p.julgm2_a             = atoi(tok);         break;
                case 18: p.suspm2_a             = atoi(tok);         break;
                case 19: p.cumprimento_meta2a   = (float)atof(tok);  break;
                case 20: p.distm2_ant           = atoi(tok);         break;
                case 21: p.julgm2_ant           = atoi(tok);         break;
                case 22: p.suspm2_ant           = atoi(tok);         break;
                case 23: p.desom2_ant           = atoi(tok);         break;
                case 24: p.cumprimento_meta2ant = (float)atof(tok);  break;
                case 25: p.distm4_a             = atoi(tok);         break;
                case 26: p.julgm4_a             = atoi(tok);         break;
                case 27: p.suspm4_a             = atoi(tok);         break;
                case 28: p.cumprimento_meta4a   = (float)atof(tok);  break;
                case 29: p.distm4_b             = atoi(tok);         break;
                case 30: p.julgm4_b             = atoi(tok);         break;
                case 31: p.suspm4_b             = atoi(tok);         break;
                case 32: p.cumprimento_meta4b   = (float)atof(tok);  break;
            }
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
    FILE *f = fopen("cvs.csv", "w");
    if (!f) { fprintf(stderr, "Erro ao criar cvs.csv\n"); return; }

    fprintf(f, CABECALHO_CSV);  /* Escreve o cabeçalho */
    for (int i = 0; i < L->Tamanho; i++)
        ESCREVER_LINHA(f, &L->Dados[i]);  /* Escreve cada registro */

    fclose(f);
    printf("   cvs.csv gerado com %d registros.\n", L->Tamanho);
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
    /* Estrutura auxiliar para agrupar dados por tribunal */
    typedef struct {
        char sigla[12];  /* Sigla do tribunal (ex: TRE-AC) */
        long jul1,  nov1,  susp1,  des1;    /* Para Meta1 */
        long jul2a, dist2a, susp2a;         /* Para Meta2A */
        long jul2an, dist2an, susp2an, des2an;  /* Para Meta2Ant */
        long jul4a, dist4a, susp4a;         /* Para Meta4A */
        long jul4b, dist4b, susp4b;         /* Para Meta4B */
    } RT;

    RT trib[200];   /* Suporta até 200 tribunais diferentes */
    int n = 0;      /* Número de tribunais encontrados */

    /* Agrupa os dados por tribunal */
    for (int i = 0; i < L->Tamanho; i++) {
        Processo *p = &L->Dados[i];

        /* Procura se já existe uma entrada para este tribunal */
        int idx = -1;
        for (int j = 0; j < n; j++)
            if (strcmp(trib[j].sigla, p->sigla_tribunal) == 0) { idx = j; break; }

        /* Se não encontrou, cria uma nova entrada */
        if (idx < 0) {
            if (n >= 200) continue;  /* Limite de tribunais atingido */
            idx = n++;
            memset(&trib[idx], 0, sizeof(RT));
            strncpy(trib[idx].sigla, p->sigla_tribunal, 11);
        }

        /* Soma os valores para cada meta */
        trib[idx].jul1    += p->julgados_2026;
        trib[idx].nov1    += p->casos_novos_2026;
        trib[idx].susp1   += p->suspensos_2026;
        trib[idx].des1    += p->dessobrestados_2026;
        trib[idx].jul2a   += p->julgm2_a;
        trib[idx].dist2a  += p->distm2_a;
        trib[idx].susp2a  += p->suspm2_a;
        trib[idx].jul2an  += p->julgm2_ant;
        trib[idx].dist2an += p->distm2_ant;
        trib[idx].susp2an += p->suspm2_ant;
        trib[idx].des2an  += p->desom2_ant;
        trib[idx].jul4a   += p->julgm4_a;
        trib[idx].dist4a  += p->distm4_a;
        trib[idx].susp4a  += p->suspm4_a;
        trib[idx].jul4b   += p->julgm4_b;
        trib[idx].dist4b  += p->distm4_b;
        trib[idx].susp4b  += p->suspm4_b;
    }

    /* Cria o arquivo resumo.csv */
    FILE *f = fopen("resumo.csv", "w");
    if (!f) { fprintf(stderr, "Erro ao criar resumo.csv\n"); return; }

    /* Escreve o cabeçalho */
    fprintf(f, "sigla_tribunal;total_julgados_2026;Meta1;Meta2A;Meta2Ant;Meta4A;Meta4B\n");

    /* Calcula e escreve as metas para cada tribunal */
    for (int i = 0; i < n; i++) {
        RT *t = &trib[i];
        double m1=0, m2a=0, m2an=0, m4a=0, m4b=0;

        /* Calcula os denominadores para cada fórmula */
        long d1   = t->nov1  + t->des1  - t->susp1;    /* Meta1 */
        long d2a  = t->dist2a  - t->susp2a;            /* Meta2A */
        long d2an = t->dist2an - t->susp2an - t->des2an;  /* Meta2Ant */
        long d4a  = t->dist4a  - t->susp4a;            /* Meta4A */
        long d4b  = t->dist4b  - t->susp4b;            /* Meta4B */

        /* Aplica as fórmulas das metas (evita divisão por zero) */
        if (d1   != 0) m1   = (double)t->jul1  / d1   * 100.0;
        if (d2a  != 0) m2a  = (double)t->jul2a / d2a  * (1000.0 / 7.0); /* Fórmula especial */
        if (d2an != 0) m2an = (double)t->jul2an/ d2an * 100.0;
        if (d4a  != 0) m4a  = (double)t->jul4a / d4a  * 100.0;
        if (d4b  != 0) m4b  = (double)t->jul4b / d4b  * 100.0;

        /* Escreve a linha no arquivo */
        fprintf(f, "%s;%ld;%.2f%%;%.2f%%;%.2f%%;%.2f%%;%.2f%%\n",
                t->sigla, t->jul1, m1, m2a, m2an, m4a, m4b);
    }
    fclose(f);
    printf("   resumo.csv gerado com %d tribunais.\n", n);
}

/* ===========================================
   FILTRAR DADOS POR MUNICÍPIO
   Cria um arquivo CSV específico com registros de um município
   =========================================== */

void filtrar_municipio(Lista *L, char *busca) {
    char nome_arq[200];
    sprintf(nome_arq, "%s.csv", busca);  /* Nome do arquivo será o nome do município */

    FILE *f = fopen(nome_arq, "w");
    if (!f) { fprintf(stderr, "Erro ao criar %s\n", nome_arq); return; }

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
    printf("   %d registro(s) encontrado(s) -> %s\n", encontrados, nome_arq);
}