# TP01 - Manipulacao de Arquivos CSV com TAD Lista

**Universidade Católica de Brasília - UCB**  
**Disciplina:** Estrutura de Dados
**Semestre:** 1º Semestre de 2026  
**Data:** Abril de 2026

## Integrantes

- MARCOS AURÉLIO MOREIRA COSTA RABELO
- PEDRO CAUA VALENTIN DE MORAES
- CARLOS EDUARDO SOARES SOUZA SANTOS
- ISAÍAS GONÇALVES DE AZEVEDO

---

## Inicio Rapido - Compilacao e Execucao

### Compilacao com GCC

**Linux/Mac:**
```bash
mkdir -p build
gcc -Wall -Wextra -std=c99 -I"include" src/Main.c src/Prop.c -o build/TP1_Estrutura_de_Dados -lm
```

**Windows (PowerShell):**
```powershell
mkdir build -Force
gcc -Wall -Wextra -std=c99 -I"include" src/Main.c src/Prop.c -o build/TP1_Estrutura_de_Dados.exe -lm
```

**Windows (CMD):**
```cmd
mkdir build
gcc -Wall -Wextra -std=c99 -I"include" src/Main.c src/Prop.c -o build/TP1_Estrutura_de_Dados.exe -lm
```

### Execucao

**Linux/Mac:**
```bash
./build/TP1_Estrutura_de_Dados
```

**Windows (PowerShell):**
```powershell
.\build\TP1_Estrutura_de_Dados.exe
```

**Windows (CMD):**
```cmd
build\TP1_Estrutura_de_Dados.exe
```

---

## Pre-requisitos

- GCC (compilador C) instalado e disponível no PATH
- Dados em formato CSV na pasta `dados/testes/`
- Os 27 arquivos de teste: `teste_TRE-AC.csv` até `teste_TRE-TO.csv`

---

## Estrutura de Arquivos

```
TP1-Estrutura-de-Dados/
├── CMakeLists.txt              - Configuração CMake
├── README.md                   - Este arquivo (documentação)
├── include/
│   └── Libs.h                  - Interface pública do TAD
├── src/
│   ├── Main.c                  - Programa principal e interface com usuário
│   └── Prop.c                  - Implementação do TAD Lista
├── dados/
│   └── testes/                 - Arquivos CSV de entrada (teste_TRE-*.csv)
└── resultados/                 - Arquivos CSV de saída (gerado automaticamente)
```

---

## Funcionalidades

1. **Leitura de multiplos arquivos CSV** - Carrega dados de 27 estados
2. **Concatenacao** - Consolida tudo em um arquivo unico (cvs.csv)
3. **Resumo por Tribunal** - Calcula metas por tribunal (resumo.csv)
4. **Filtro por Estado** - Gera resumo filtrado por UF (resumo_XX.csv)
5. **Busca por Municipio** - Filtra dados por municipio (MUNICIPIO.csv)
6. **Calculo de Metas** - Automatico de 5 metas judiciais

### Validacoes Implementadas

- UF valida (2 letras, estado brasileiro)
- Municipio valido (letras, espacos, hifens, apostrofos)
- Deteccao automatica de separador CSV (`;` ou `,`)
- Alocacao dinamica com redimensionamento automatico

---

# Documentacao Tecnica Detalhada

## 1. Objetivo do Projeto

O projeto implementa um sistema em linguagem C que manipula dados de processos judiciais da Justiça Eleitoral brasileira, armazenados em formato CSV (Comma Separated Values).

### Funcionalidades Principais:
1. Leitura de multiplos arquivos CSV
2. Consolidacao de dados em arquivo unico
3. Geracao de resumo de estatísticas por tribunal
4. Filtro de dados por estado (UF)
5. Busca e filtro por municipio

---

## 2. Estrutura de Dados - TAD Lista

### Definicao da Estrutura Processo

Representa um registro unico de um orgao julgador da Justica Eleitoral:

```c
typedef struct {
    // Identificadores (campos de texto)
    char  sigla_tribunal[12];      // Ex: TJSP, TRF1
    char  procedimento[100];       // Tipo de procedimento
    char  ramo_justica[60];        // Ramo da justica
    char  sigla_grau[10];          // Grau de jurisdicao
    char  uf_oj[5];                // UF do orgao julgador
    char  municipio_oj[100];       // Municipio do orgao
    char  id_ultimo_oj[60];        // ID do ultimo orgao
    char  nome[200];               // Nome do orgao
    char  mesano_cnm1[20];         // Mes/Ano (Meta CNM 1)
    char  mesano_sent[20];         // Mes/Ano (Sentenca)
    
    // Dados numericos - Meta 1
    int   casos_novos_2026;        // Processos distribuidos
    int   julgados_2026;           // Processos julgados
    int   prim_sent2026;           // Primeira sentenca
    int   suspensos_2026;          // Processos suspensos
    int   dessobrestados_2026;     // Processos dessobrestados
    float cumprimento_meta1;       // Percentual de cumprimento
    
    // Dados numericos - Meta 2A
    int   distm2_a;                // Distribuidos 2A
    int   julgm2_a;                // Julgados 2A
    int   suspm2_a;                // Suspensos 2A
    float cumprimento_meta2a;      // Percentual de cumprimento
    
    // Dados numericos - Meta 2Ant
    int   distm2_ant;              // Distribuidos 2Ant
    int   julgm2_ant;              // Julgados 2Ant
    int   suspm2_ant;              // Suspensos 2Ant
    int   desom2_ant;              // Dessobrestados 2Ant
    float cumprimento_meta2ant;    // Percentual de cumprimento
    
    // Dados numericos - Meta 4A
    int   distm4_a;                // Distribuidos 4A
    int   julgm4_a;                // Julgados 4A
    int   suspm4_a;                // Suspensos 4A
    float cumprimento_meta4a;      // Percentual de cumprimento
    
    // Dados numericos - Meta 4B
    int   distm4_b;                // Distribuidos 4B
    int   julgm4_b;                // Julgados 4B
    int   suspm4_b;                // Suspensos 4B
    float cumprimento_meta4b;      // Percentual de cumprimento
} Processo;
```

### Definicao da Estrutura Lista

Implementa uma lista dinamica para armazenar multiplos processos:

```c
typedef struct {
    Processo *Dados;      // Vetor dinamico de processos
    int Tamanho;          // Quantidade de processos armazenados
    int Capacidade;       // Capacidade total de alocacao
} Lista;
```

**Caracteristicas:**
- Aloca memoria dinamicamente
- Redimensiona automaticamente quando necessario (capacity doubling)
- Suporta insercao eficiente de novos elementos

---

## 3. Funcionalidades Implementadas

### 3.1 Leitura de Arquivos CSV

**Funcao:** `void carregar_arquivo(Lista *L, const char *nome_arquivo)`

**Descricao:**
- Abre arquivo CSV e le linha por linha
- Detecta automaticamente o separador (`;` ou `,`)
- Processa cada linha e converte valores para os tipos apropriados
- Adiciona o registro a lista dinamica

**Validacao:**
- Verifica se arquivo existe
- Valida quantidade de colunas
- Converte strings em inteiros e floats

### 3.2 Concatenacao de Arquivos

**Funcao:** `void concatenar_arquivos(Lista *L)`

**Descricao:**
- Consolida todos os dados carregados em um unico arquivo CSV
- Nome do arquivo: `resultados/cvs.csv`
- Gera cabecalho com os 33 campos
- Escreve um registro por linha
- **Delimitador:** `,` (vírgula) para compatibilidade

**Formato de Saida (cvs.csv):**
```
sigla_tribunal,procedimento,ramo_justica,...[30 campos adicionais]
TJSP,Civil,Justica Comum,...
TRF1,Criminal,Justica Federal,...
```

### 3.3 Geracao de Resumo por Tribunal

**Funcao:** `void gerar_resumo(Lista *L)`

**Descricao:**
- Agrupa dados por `sigla_tribunal`
- Calcula somatorios para cada metrica
- Aplica formulas de calculo das 5 metas
- Nome do arquivo: `resultados/resumo.csv`

**Formulas de Calculo:**

#### Meta 1 (Velocidade de Baixa de Processos)
$$\text{Meta 1} = \frac{\sum \text{julgados\_2026}}{\sum \text{casos\_novos} + \sum \text{dessobrestados} - \sum \text{suspensos}} \times 100\%$$

#### Meta 2A (Velocidade com Requisitos Especificos)
$$\text{Meta 2A} = \frac{\sum \text{julgados\_2a}}{\sum \text{distribuidos\_2a} - \sum \text{suspensos\_2a}} \times \frac{1000}{7}$$

#### Meta 2Ant (Velocidade para Processos Antigos)
$$\text{Meta 2Ant} = \frac{\sum \text{julgados\_2ant}}{\sum \text{distribuidos\_2ant} - \sum \text{suspensos\_2ant} - \sum \text{dessobrestados\_2ant}} \times 100\%$$

#### Meta 4A e 4B (Fila Inicial)
$$\text{Meta 4} = \frac{\sum \text{julgados\_4}}{\sum \text{distribuidos\_4} - \sum \text{suspensos\_4}} \times 100\%$$

**Saida (resumo.csv):**
```
sigla_tribunal;total_julgados_2026;Meta1;Meta2A;Meta2Ant;Meta4A;Meta4B
TJSP;5000;85.50%;145.25%;92.30%;78.40%;88.90%
TRF1;3200;79.80%;138.50%;88.20%;82.10%;91.20%
```

**Delimitador:** `;` (ponto-e-vírgula)

### 3.4 Resumo Filtrado por Estado

**Funcao:** `void gerar_resumo_por_estado(Lista *L, const char *uf)`

**Descricao:**
- Filtra apenas registros de um estado especifico
- Aplica mesmo algoritmo de calculo de metas
- Nome do arquivo: `resultados/resumo_XX.csv` (onde XX = UF)

**Validacao de UF:**
- Valida se UF e brasileira valida
- Aceita apenas 27 UFs reconhecidas
- Case-insensitive (aceita "sp", "SP", "Sp")

**UFs Aceitas:**
AC, AL, AP, AM, BA, CE, DF, ES, GO, MA, MT, MS, MG, PA, PB, PR, PE, PI, RJ, RN, RS, RO, RR, SC, SP, SE, TO

### 3.5 Busca e Filtro por Municipio

**Funcao:** `void filtrar_municipio(Lista *L, const char *nome_busca)`

**Descricao:**
- Busca registros correspondentes ao municipio informado
- Case-insensitive
- Nome do arquivo: `resultados/{MUNICIPIO}.csv`

**Validacao:**
- Aceita letras, espacos, hifens e apostrofos
- Rejeita numeros e caracteres especiais
- Suporta acentuacao

**Dica para Usuario:**
Se nenhum resultado for encontrado e a busca contiver apenas ASCII, o sistema sugere usar a grafia oficial com acentuacao.

---

## 4. Organizacao do Codigo

### 4.1 Estrutura de Arquivos do Projeto

```
TP1-Estrutura-de-Dados/
├── CMakeLists.txt              # Configuracao CMake
├── README.md                   # Este documento
├── include/
│   └── Libs.h                  # Interface publica do TAD
├── src/
│   ├── Main.c                  # Programa principal
│   └── Prop.c                  # Implementacao do TAD
├── dados/
│   └── testes/                 # CSVs de entrada
└── resultados/                 # CSVs de saida (gerado)
```

### 4.2 Responsabilidades de Cada Arquivo

#### `include/Libs.h`
- Define estruturas de dados (Processo, Lista)
- Declara funcoes publicas
- Interface entre modulos

#### `src/Main.c`
- Funcao `main()` - entrada principal
- Interface com usuario (input/output)
- Validacao de entrada do usuario
- Normalizacao de dados de entrada
- Resolucao de caminhos de arquivo
- Tratamento de erros do usuario

#### `src/Prop.c`
- Implementacao completa do TAD Lista
- Funcoes de manipulacao de arquivos
- Calculo de estatisticas
- Funcoes auxiliares (parsing, conversao, validacao)
- Macros para formatacao de saida

---

## 5. Fluxo de Execucao

```
INICIO
  |
Ler 27 arquivos CSV de dados
  |
Armazenar na Lista dinamica
  |
+-- Opcao 1: Gerar cvs.csv (concatenacao)
|
+-- Opcao 2: Gerar resumo.csv (todas as UFs)
|
+-- Opcao 3: Solicitar UF
|   +-- Validar entrada (se invalida, repetir)
|   +-- Gerar resumo_{UF}.csv
|
+-- Opcao 4: Solicitar Municipio
|   +-- Validar entrada (se invalida, repetir)
|   +-- Gerar {MUNICIPIO}.csv
|
+-- Liberar memoria
  |
FIM
```

---

## 6. Algoritmos Principais

### 6.1 Leitura de CSV com Parsing

**Funcao:** `carregar_arquivo()`

```
PARA CADA LINHA NO ARQUIVO
    CRIAR novo Processo
    PARA CADA COLUNA NA LINHA
        EXTRAIR token (campo)
        REMOVER espacos em branco
        CONVERTER para tipo apropriado
        ARMAZENAR em Processo
    FIM
    SE (quantidade de colunas > 5)
        ADICIONAR Processo a Lista
    FIM
FIM
```

**Deteccao de Separador:**
- Verifica presenca de `;` -> usa `;`
- Caso contrario -> usa `,`

### 6.2 Calculo de Resumo por Tribunal

**Funcao:** `gerar_resumo()`

```
CRIAR array de estruturas RT (Resumo de Tribunal)
n = 0

PARA CADA Processo P NA LISTA
    PROCURAR tribunal com sigla == P.sigla_tribunal
    SE nao encontrado
        CRIAR novo resumo de tribunal
        n++
    FIM
    ACUMULAR valores de P no resumo do tribunal
FIM

PARA CADA tribunal T
    CALCULAR Meta 1 usando formula
    CALCULAR Meta 2A usando formula
    CALCULAR Meta 2Ant usando formula
    CALCULAR Meta 4A usando formula
    CALCULAR Meta 4B usando formula
    ESCREVER linha em arquivo
FIM
```

### 6.3 Busca de Municipio

**Funcao:** `filtrar_municipio()`

```
PARA CADA Processo P NA LISTA
    SE P.municipio_oj == busca (case-insensitive)
        ESCREVER P em arquivo
        encontrados++
    FIM
FIM

SE encontrados == 0 E busca contem apenas ASCII
    EXIBIR dica sobre acentuacao
FIM
```

---

## 7. Tratamento de Erros

O programa implementa validacoes em multiplos niveis:

### Validacao de UF
- Maximo 2 caracteres
- Apenas letras
- Existencia na lista de UFs brasileiras
- Case-insensitive

### Validacao de Municipio
- Minimo 1 caractere
- Contem pelo menos uma letra
- Aceita: letras, espacos, hifens, apostrofos
- Rejeita: numeros, caracteres especiais

### Validacao de Arquivo
- Verifica existencia do arquivo
- Detecta separador automaticamente
- Valida quantidade de colunas
- Converte tipos com seguranca

### Gestao de Memoria
- Alocacao dinamica com verificacao
- Reallocation automatica quando capacidade e atingida
- Liberacao propria de recursos

---

## 8. Delimitadores CSV - Clarificação

**Importante:** O projeto utiliza delimitadores diferentes para **entrada** e **saída**:

| Tipo de Arquivo | Delimitador | Descrição |
|---|---|---|
| **Entrada** (teste_TRE-*.csv) | `;` (ponto-e-vírgula) | Arquivos de entrada fornecidos |
| **Concatenação** (cvs.csv) | `,` (vírgula) | Todos os dados consolidados |
| **Resumo** (resumo.csv) | `;` (ponto-e-vírgula) | Metas por tribunal |
| **Filtro Estado** (resumo_XX.csv) | `;` (ponto-e-vírgula) | Metas por UF |
| **Filtro Município** ({MUNICIPIO}.csv) | `,` (vírgula) | Dados filtrados por município |

**Detecção Automática:** O código detecta automaticamente o separador dos arquivos de entrada.

---

## 9. Exemplo de Entrada e Saida

### Entrada (teste_TRE-SP.csv)
```
sigla_tribunal;procedimento;ramo_justica;sigla_grau;...;julgados_2026;...
TJSP;Civil;Justica Comum;1;...;150;...
TJSP;Criminal;Justica Comum;1;...;200;...
TRF1;Civil;Justica Federal;1;...;120;...
```
**Delimitador:** `;` (ponto-e-vírgula)

### Saida 1 - Concatenacao (cvs.csv)
```
sigla_tribunal,procedimento,ramo_justica,sigla_grau,...
TJSP,Civil,Justica Comum,1,...
TJSP,Criminal,Justica Comum,1,...
TRF1,Civil,Justica Federal,1,...
```
**Delimitador:** `,` (vírgula)

### Saida 2 - Resumo (resumo.csv)
```
sigla_tribunal;total_julgados_2026;Meta1;Meta2A;Meta2Ant;Meta4A;Meta4B
TJSP;350;85.50%;145.25%;92.30%;78.40%;88.90%
TRF1;120;79.80%;138.50%;88.20%;82.10%;91.20%
```
**Delimitador:** `;` (ponto-e-vírgula)

---

## 10. Testes Recomendados

### Teste 1: Carga de Dados
- Carregar todos os 27 arquivos
- Verificar se numero de registros esta correto

### Teste 2: Concatenacao
- Verificar se cvs.csv foi gerado
- Comparar numero de linhas com soma dos arquivos

### Teste 3: Resumo Geral
- Verificar se resumo.csv foi gerado
- Validar numero de tribunais
- Verificar se metas estao entre 0% e 200%

### Teste 4: Filtro por UF
- Testar com UF valida: SP, RJ, etc.
- Testar com UF invalida: XX, 12
- Testar com entrada vazia

### Teste 5: Filtro por Municipio
- Testar com municipio existente
- Testar com municipio inexistente
- Testar com acentuacao
- Testar com entrada invalida

### Teste 6: Tratamento de Erros
- Remover arquivos de entrada e verificar mensagens de erro
- Testar com disco cheio (simular)
- Testar com permissoes de arquivo limitadas

---

## 11. Conceitos de Programacao Utilizados

### Estruturas de Dados
- TAD Lista: implementacao com alocacao dinamica
- Estruturas (struct): Processo, Lista, RT
- Arrays dinamicos: redimensionamento automatico

### Tecnicas de Programacao
- I/O de Arquivos: leitura e escrita de CSV
- String Handling: parsing de dados
- Validacao de entrada: tratamento de erros
- Algoritmos: ordenacao (implicita), busca, agregacao

### Padroes de Design
- TAD (Tipo Abstrato de Dados): encapsulamento de funcionamento da lista
- Separacao de responsabilidades: Main.c vs Prop.c
- Tratamento de erros: verificacao em multiplas camadas

---

## 12. Referencias e Recursos

### Funcoes Padrao de C Utilizadas
- `fopen()`, `fclose()`: I/O de arquivos
- `fgets()`, `fprintf()`: leitura e escrita
- `malloc()`, `realloc()`, `free()`: gestao de memoria
- `atoi()`, `atof()`: conversao de tipos
- `strcpy()`, `strcmp()`, `strlen()`: manipulacao de strings
- `toupper()`, `tolower()`: transformacao de caracteres

### Estrutura do Projeto
O projeto segue os padroes de:
- Codigo modular e reutilizavel
- TAD bem definido
- Documentacao clara
- Tratamento de erros robusto

---

## 13. Conclusao

Este projeto implementa um sistema completo de manipulacao de dados CSV em C, utilizando:
- TAD Lista para armazenamento dinamico
- Leitura inteligente de multiplos arquivos
- Calculo de estatisticas complexas
- Interface amigavel com o usuario
- Tratamento robusto de erros




