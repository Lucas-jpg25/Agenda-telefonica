# Agenda Telefonica - ABB em C

Sistema de linha de comando (CLI) em C para gerenciar contatos telefonicos, utilizando **Arvore Binaria de Busca (ABB)** como estrutura de dados principal.

---

## 1. Descricao do problema

Gerenciar uma agenda de contatos (nome, telefone, e-mail) de forma eficiente, garantindo listagem sempre em ordem alfabetica e busca rapida por nome, sem varrer todos os registros um a um. O sistema persiste os dados em disco e os recarrega automaticamente a cada execucao.

---

## 2. Estrutura de dados utilizada: ABB

| Operacao | ABB (caso medio) | Array desordenado |
|---|---|---|
| Insercao | O(log n) | O(1) |
| Busca exata | O(log n) | O(n) |
| Listagem ordenada | O(n) em-ordem | O(n log n) com sort |
| Remocao | O(log n) | O(n) |

**Por que ABB e nao lista encadeada?**
A lista exigiria ordenacao a cada insercao ou percurso completo em cada busca. A ABB mantem os dados ordenados estruturalmente: o percurso em-ordem ja produz a saida alfabetica sem custo extra de ordenacao.

**Por que nao fila ou pilha?**
Fila e pilha sao estruturas FIFO/LIFO e nao fazem sentido para um problema de busca por chave (nome). A ABB e a estrutura correta aqui.

**Tratamento de duplicatas:**
Contatos com o mesmo nome (ex: duas "Ana Silva") sao permitidos. Duplicatas sao inseridas sempre a direita, e identificadas pelo par (nome + telefone) nas operacoes de busca, edicao e remocao.

---

## 3. Funcionalidades

| Opcao | Descricao |
|---|---|
| 1 | Adicionar contato (nome, telefone, e-mail) |
| 2 | Listar todos em ordem alfabetica |
| 3 | Listar agrupado por inicial (A, B, C...) |
| 4 | Buscar por nome exato (localiza duplicatas) |
| 5 | Editar contato existente |
| 6 | Remover contato (com confirmacao s/n) |
| 7 | Salvar manualmente |
| 8 | Salvar e sair |

O menu exibe o total de contatos cadastrados em tempo real.

---

## 4. Decisoes de implementacao

**Edicao segura:**
A edicao remove o no antigo e reinsere com os novos dados. Isso garante que a propriedade da ABB seja sempre preservada, mesmo quando o nome do contato muda de posicao na ordem alfabetica.

**Confirmacao antes de remover:**
A remocao exige confirmacao (s/n) apos localizar o contato, evitando exclusoes acidentais.

**Contador em tempo real:**
A funcao `contarNos()` percorre a arvore e exibe o total no cabecalho do menu a cada interacao.

---

## 5. Formato do arquivo de persistencia

Arquivo: `agenda.csv`
Formato: `nome;telefone;email` — um contato por linha.

```
# nome;telefone;email
Ana Silva;(11)99999-0001;ana@email.com
Carlos Souza;(21)98888-0002;
Maria Lima;(31)97777-0003;maria@email.com
```

**Por que CSV com `;`?**
Nomes podem conter virgulas, entao `;` e um separador mais seguro. Alem disso, e legivel em qualquer editor de texto ou planilha e facil de parsear com `strtok`.

**Por que a linha `#`?**
Autodocumenta o arquivo sem custo: o carregador ignora qualquer linha que comece com `#`, entao o cabecalho nunca vira um contato fantasma.

**Por que salvar em pre-ordem?**
Salvar em pre-ordem e recarregar na mesma sequencia reconstroi a arvore com a mesma forma, evitando que ela degenere em lista linear. O pior caso da ABB ocorre quando todos os nos chegam ja ordenados, o que aconteceria se salvassemos em em-ordem.

---

## 6. Como compilar e executar

```bash
gcc -Wall -Wextra -std=c99 -o agenda agenda.c
./agenda
```

Requer apenas a biblioteca padrao C (C99 ou superior). Compativel com GCC no Windows (Dev-C++, MinGW) e Linux/macOS.

---

## 7. Limitacoes conhecidas

- **Sem balanceamento automatico**: se os dados forem inseridos em ordem alfabetica estrita por uma fonte externa, a arvore pode ficar desbalanceada. Solucao futura: AVL ou Red-Black Tree.
- **Tamanho fixo dos campos**: nome (99 chars), telefone (19 chars), e-mail (99 chars). Alteravel pelas constantes `TAM_NOME`, `TAM_TELEFONE` e `TAM_EMAIL` no topo do arquivo.
- **Sem validacao de formato**: o sistema aceita qualquer string como telefone ou e-mail.
- **Arquivo unico**: todos os contatos ficam em `agenda.csv` no diretorio de execucao.
