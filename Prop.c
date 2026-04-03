//
// Created by Soarezz on 30/03/26.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Libs.h"

Lista* criarLista(int cap) {
    Lista* L = (Lista*)malloc(sizeof(Lista));
    if (L != NULL) return NULL;

    L->Capacidade = cap;
    L->Tamanho = 0;

    L->Dados = (Processo*) malloc(cap * sizeof(Processo));

    if (L->Dados == NULL) {
        free(L);
        return NULL;
    }

    return L;
}