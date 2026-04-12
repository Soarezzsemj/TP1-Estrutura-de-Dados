# Arquitetura do projeto

## Estrutura de pastas

- `src/`: código-fonte de compilação (`Main.c` e `Prop.c`)
- `include/`: headers públicos (`Libs.h`)
- `dados/testes/`: local recomendado para os CSVs de entrada
- `resultados/`: saída dos CSVs gerados pelo programa
- `docs/`: documentação do projeto

## Fluxo esperado

1. Coloque os arquivos `teste_TRE-*.csv` em `dados/testes/`.
2. Execute o projeto normalmente.
3. Os resultados serão gravados em `resultados/`.

## Compatibilidade

O carregamento dos CSVs tenta, nesta ordem:

1. `dados/testes/`
2. `testes/`
3. raiz do projeto
