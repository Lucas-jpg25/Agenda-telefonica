#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* =============================================================
 * CONSTANTES - altere aqui para mudar limites em todo o codigo
 * ============================================================= */
#define TAM_NOME      100
#define TAM_TELEFONE   20
#define TAM_EMAIL     100
#define TAM_LINHA     250   /* nome + telefone + email + separadores */

/* =============================================================
 * ESTRUTURA DO NO
 * ============================================================= */
typedef struct No {
    char nome[TAM_NOME];
    char telefone[TAM_TELEFONE];
    char email[TAM_EMAIL];
    struct No* esq;
    struct No* dir;
} No;

/* =============================================================
 * UTILITARIOS DE STRING
 * ============================================================= */

/* Remove \n e \r do final da string */
void removerNovaLinha(char* str) {
    str[strcspn(str, "\r\n")] = 0;
}

/* Comparacao case-insensitive letra a letra */
int compararSemCaixa(const char* s1, const char* s2) {
    while (*s1 && *s2) {
        char c1 = tolower((unsigned char)*s1);
        char c2 = tolower((unsigned char)*s2);
        if (c1 != c2) return c1 - c2;
        s1++; s2++;
    }
    return tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
}

/*
 * Le uma linha do stdin com seguranca.
 * Se fgets falhar (EOF, erro), garante buf vazio - sem lixo no buffer.
 */
void lerLinha(char* buf, int tamanho) {
    if (fgets(buf, tamanho, stdin)) {
        removerNovaLinha(buf);
    } else {
        buf[0] = '\0';   /* falha: garante string vazia em vez de lixo */
    }
}

/* =============================================================
 * OPERACOES DA ARVORE BINARIA DE BUSCA
 * ============================================================= */

/* Aloca e inicializa um novo no */
No* criarNo(const char* nome, const char* telefone, const char* email) {
    No* novo = (No*)malloc(sizeof(No));
    if (!novo) { fprintf(stderr, "Erro: memoria insuficiente.\n"); exit(1); }
    strncpy(novo->nome,     nome,     TAM_NOME     - 1); novo->nome[TAM_NOME - 1]         = '\0';
    strncpy(novo->telefone, telefone, TAM_TELEFONE - 1); novo->telefone[TAM_TELEFONE - 1] = '\0';
    strncpy(novo->email,    email,    TAM_EMAIL    - 1); novo->email[TAM_EMAIL - 1]        = '\0';
    novo->esq = novo->dir = NULL;
    return novo;
}

/*
 * INSERCAO
 * Nomes iguais vao para a direita, permitindo duplicatas
 * (dois "Ana Silva" com telefones diferentes, por exemplo).
 */
No* inserir(No* raiz, const char* nome, const char* telefone, const char* email) {
    if (!raiz) return criarNo(nome, telefone, email);
    if (compararSemCaixa(nome, raiz->nome) < 0)
        raiz->esq = inserir(raiz->esq, nome, telefone, email);
    else
        raiz->dir = inserir(raiz->dir, nome, telefone, email);
    return raiz;
}

/* Retorna o no com o menor valor (mais a esquerda) da subarvore */
No* encontrarMinimo(No* raiz) {
    while (raiz->esq) raiz = raiz->esq;
    return raiz;
}

/*
 * REMOCAO
 * Identifica o no pelo par (nome + telefone) para tratar duplicatas.
 * Implementa os tres casos classicos da ABB:
 *   Caso 1: no folha          -> remove diretamente
 *   Caso 2: um filho          -> substitui pelo filho
 *   Caso 3: dois filhos       -> substitui pelo sucessor (minimo da direita)
 */
No* remover(No* raiz, const char* nome, const char* telefone, int* removido) {
    if (!raiz) return NULL;

    int cmp = compararSemCaixa(nome, raiz->nome);

    if (cmp < 0) {
        raiz->esq = remover(raiz->esq, nome, telefone, removido);
    } else if (cmp > 0) {
        raiz->dir = remover(raiz->dir, nome, telefone, removido);
    } else {
        if (strcmp(telefone, raiz->telefone) == 0) {
            /* No encontrado */
            *removido = 1;
            if (!raiz->esq) { No* tmp = raiz->dir; free(raiz); return tmp; }
            if (!raiz->dir) { No* tmp = raiz->esq; free(raiz); return tmp; }
            /* Dois filhos: copia dados do sucessor e o remove */
            No* suc = encontrarMinimo(raiz->dir);
            strncpy(raiz->nome,     suc->nome,     TAM_NOME     - 1); raiz->nome[TAM_NOME - 1]         = '\0';
            strncpy(raiz->telefone, suc->telefone, TAM_TELEFONE - 1); raiz->telefone[TAM_TELEFONE - 1] = '\0';
            strncpy(raiz->email,    suc->email,    TAM_EMAIL    - 1); raiz->email[TAM_EMAIL - 1]        = '\0';
            raiz->dir = remover(raiz->dir, suc->nome, suc->telefone, removido);
        } else {
            /* Mesmo nome, telefone diferente: continua a direita */
            raiz->dir = remover(raiz->dir, nome, telefone, removido);
        }
    }
    return raiz;
}

/*
 * BUSCA SILENCIOSA
 * Retorna o ponteiro para o no identificado por (nome + telefone),
 * sem nenhum efeito colateral visual. Usada internamente por editar()
 * e pelo case de edicao no menu para verificar existencia antes de agir.
 */
No* buscarNo(No* raiz, const char* nome, const char* telefone) {
    if (!raiz) return NULL;
    int cmp = compararSemCaixa(nome, raiz->nome);
    if (cmp < 0) return buscarNo(raiz->esq, nome, telefone);
    if (cmp > 0) return buscarNo(raiz->dir, nome, telefone);
    /* Nome bate: verifica telefone */
    if (strcmp(telefone, raiz->telefone) == 0) return raiz;
    /* Mesmo nome, telefone diferente: continua a direita */
    return buscarNo(raiz->dir, nome, telefone);
}

/*
 * EDICAO
 * Estrategia correta: remove o no antigo primeiro, depois reinsere
 * com os novos dados. Isso garante que a propriedade da ABB seja
 * sempre preservada, independente de o nome mudar ou nao.
 *
 * Fluxo:
 *   1. Verifica se o contato existe (via buscarNo)
 *   2. Remove pelo par antigo
 *   3. Reinsere com os dados novos
 */
No* editar(No* raiz, const char* nomeAntigo, const char* telAntigo,
           const char* nomeNovo,  const char* telNovo, const char* emailNovo,
           int* editado) {
    if (!buscarNo(raiz, nomeAntigo, telAntigo)) return raiz; /* nao encontrado */

    int dummy = 0;
    raiz = remover(raiz, nomeAntigo, telAntigo, &dummy);
    raiz = inserir(raiz, nomeNovo, telNovo, emailNovo);
    *editado = 1;
    return raiz;
}

/*
 * BUSCA EXATA (por nome completo)
 * Percorre a arvore e imprime todos os contatos com o nome informado.
 * Retorna a quantidade de contatos encontrados.
 */
int buscarExato(No* raiz, const char* nome) {
    if (!raiz) return 0;
    int cont = 0;
    int cmp = compararSemCaixa(nome, raiz->nome);

    if (cmp < 0) {
        cont += buscarExato(raiz->esq, nome);
    } else if (cmp > 0) {
        cont += buscarExato(raiz->dir, nome);
    } else {
        printf("  Nome:     %s\n", raiz->nome);
        printf("  Telefone: %s\n", raiz->telefone);
        if (raiz->email[0] != '\0')
            printf("  Email:    %s\n", raiz->email);
        printf("  --------------------\n");
        cont++;
        /* Continua a direita: pode haver duplicatas com o mesmo nome */
        cont += buscarExato(raiz->dir, nome);
    }
    return cont;
}

/*
 * LISTAGEM EM-ORDEM (alfabetica)
 * O percurso esq -> raiz -> dir produz saida ordenada naturalmente.
 */
void listarEmOrdem(No* raiz) {
    if (!raiz) return;
    listarEmOrdem(raiz->esq);
    printf("  %-30s  %-15s  %s\n", raiz->nome, raiz->telefone, raiz->email);
    listarEmOrdem(raiz->dir);
}

/*
 * AGRUPAMENTO POR INICIAL
 * Mesmo percurso em-ordem; imprime cabecalho [ A ], [ B ]...
 * sempre que a primeira letra do nome muda.
 */
void listarPorInicial(No* raiz, char* letraAtual) {
    if (!raiz) return;
    listarPorInicial(raiz->esq, letraAtual);
    char inicial = toupper((unsigned char)raiz->nome[0]);
    if (inicial != *letraAtual) {
        *letraAtual = inicial;
        printf("\n  [ %c ]\n", inicial);
    }
    printf("    %-28s  %-15s  %s\n", raiz->nome, raiz->telefone, raiz->email);
    listarPorInicial(raiz->dir, letraAtual);
}

/* Libera toda a memoria da arvore (pos-ordem) */
void liberarArvore(No* raiz) {
    if (!raiz) return;
    liberarArvore(raiz->esq);
    liberarArvore(raiz->dir);
    free(raiz);
}

/* =============================================================
 * PERSISTENCIA EM ARQUIVO CSV
 * Formato: nome;telefone;email  (uma linha por contato)
 * Linhas iniciadas com '#' sao comentarios e ignoradas na leitura.
 * Salva em pre-ordem para reconstruir a arvore com a mesma forma,
 * evitando a degeneracao para lista linear na proxima execucao.
 * ============================================================= */

void salvarNos(No* raiz, FILE* arq) {
    if (!raiz) return;
    fprintf(arq, "%s;%s;%s\n", raiz->nome, raiz->telefone, raiz->email);
    salvarNos(raiz->esq, arq);
    salvarNos(raiz->dir, arq);
}

void salvarDados(No* raiz, const char* caminho) {
    FILE* arq = fopen(caminho, "w");
    if (!arq) { fprintf(stderr, "Erro ao salvar '%s'.\n", caminho); return; }
    fprintf(arq, "# nome;telefone;email\n");   /* cabecalho autodocumentado */
    salvarNos(raiz, arq);
    fclose(arq);
    printf("  Dados salvos em '%s'.\n", caminho);
}

No* carregarDados(No* raiz, const char* caminho) {
    FILE* arq = fopen(caminho, "r");
    if (!arq) return raiz;   /* primeira execucao: arquivo ainda nao existe */

    char linha[TAM_LINHA];
    while (fgets(linha, sizeof(linha), arq)) {
        removerNovaLinha(linha);
        if (linha[0] == '\0') continue;  /* linha vazia */
        if (linha[0] == '#')  continue;  /* comentario/cabecalho */

        char* nome     = strtok(linha, ";");
        char* telefone = strtok(NULL,  ";");
        char* email    = strtok(NULL,  ";");

        if (nome && telefone)
            raiz = inserir(raiz, nome, telefone, email ? email : "");
    }
    fclose(arq);
    return raiz;
}

/* =============================================================
 * MENU E INTERACAO
 * ============================================================= */

void imprimirMenu(int total) {
    printf("\n+--------------------------------------+\n");
    printf("|      AGENDA TELEFONICA - ABB         |\n");
    printf("|         Contatos: %-3d                |\n", total);
    printf("+--------------------------------------+\n");
    printf("|  1 - Adicionar contato               |\n");
    printf("|  2 - Listar todos (alfabetica)       |\n");
    printf("|  3 - Listar por inicial (A, B, C...) |\n");
    printf("|  4 - Buscar contato (nome exato)     |\n");
    printf("|  5 - Editar contato                  |\n");
    printf("|  6 - Remover contato                 |\n");
    printf("|  7 - Salvar                          |\n");
    printf("|  8 - Salvar e Sair                   |\n");
    printf("+--------------------------------------+\n");
    printf("Escolha: ");
}

int lerOpcao(void) {
    int opcao;
    if (scanf("%d", &opcao) != 1) {
        while (getchar() != '\n');
        return -1;
    }
    getchar();   /* consome o \n restante do buffer */
    return opcao;
}

/* =============================================================
 * MAIN
 * ============================================================= */

/* Conta o total de nos da arvore — usado para exibir no menu */
int contarNos(No* raiz) {
    if (!raiz) return 0;
    return 1 + contarNos(raiz->esq) + contarNos(raiz->dir);
}

int main(void) {
    const char* ARQUIVO = "agenda.csv";
    No* raiz = NULL;
    int opcao;

    raiz = carregarDados(raiz, ARQUIVO);

    do {
        imprimirMenu(contarNos(raiz));
        opcao = lerOpcao();

        switch (opcao) {

            /* -- 1. ADICIONAR -------------------------------- */
            case 1: {
                char nome[TAM_NOME], telefone[TAM_TELEFONE], email[TAM_EMAIL];
                printf("  Nome:     "); lerLinha(nome,     sizeof(nome));
                printf("  Telefone: "); lerLinha(telefone, sizeof(telefone));
                printf("  Email:    "); lerLinha(email,    sizeof(email));

                if (nome[0] == '\0' || telefone[0] == '\0') {
                    printf("  Nome e telefone sao obrigatorios.\n");
                    break;
                }
                raiz = inserir(raiz, nome, telefone, email);
                printf("  Contato adicionado!\n");
                break;
            }

            /* -- 2. LISTAR TODOS ----------------------------- */
            case 2: {
                if (!raiz) { printf("  Agenda vazia.\n"); break; }
                printf("\n  %-30s  %-15s  %s\n", "NOME", "TELEFONE", "EMAIL");
                printf("  -----------------------------------------------------------\n");
                listarEmOrdem(raiz);
                break;
            }

            /* -- 3. LISTAR POR INICIAL ----------------------- */
            case 3: {
                if (!raiz) { printf("  Agenda vazia.\n"); break; }
                char letraAtual = '\0';
                listarPorInicial(raiz, &letraAtual);
                printf("\n");
                break;
            }

            /* -- 4. BUSCA EXATA ------------------------------ */
            case 4: {
                char nome[TAM_NOME];
                printf("  Nome para buscar: "); lerLinha(nome, sizeof(nome));
                if (nome[0] == '\0') { printf("  Nome nao pode ser vazio.\n"); break; }
                printf("\n");
                int qtd = buscarExato(raiz, nome);
                if (qtd == 0)
                    printf("  Nenhum contato encontrado com o nome '%s'.\n", nome);
                else
                    printf("  %d contato(s) encontrado(s).\n", qtd);
                break;
            }

            /* -- 5. EDITAR ----------------------------------- */
            case 5: {
                char nomeAntigo[TAM_NOME], telAntigo[TAM_TELEFONE];
                printf("  Nome atual do contato:     "); lerLinha(nomeAntigo, sizeof(nomeAntigo));
                printf("  Telefone atual do contato: "); lerLinha(telAntigo,  sizeof(telAntigo));

                /* buscarNo() centraliza a logica de busca - sem codigo duplicado */
                No* noAtual = buscarNo(raiz, nomeAntigo, telAntigo);
                if (!noAtual) {
                    printf("  Contato nao encontrado.\n");
                    break;
                }

                char nomeNovo[TAM_NOME], telNovo[TAM_TELEFONE], emailNovo[TAM_EMAIL];

                printf("  Novo nome     (Enter = manter '%s'): ", nomeAntigo);
                lerLinha(nomeNovo, sizeof(nomeNovo));
                if (nomeNovo[0] == '\0') strncpy(nomeNovo, nomeAntigo, TAM_NOME);

                printf("  Novo telefone (Enter = manter '%s'): ", telAntigo);
                lerLinha(telNovo, sizeof(telNovo));
                if (telNovo[0] == '\0') strncpy(telNovo, telAntigo, TAM_TELEFONE);

                printf("  Novo email    (Enter = manter): ");
                lerLinha(emailNovo, sizeof(emailNovo));
                if (emailNovo[0] == '\0') strncpy(emailNovo, noAtual->email, TAM_EMAIL);

                int editado = 0;
                raiz = editar(raiz, nomeAntigo, telAntigo, nomeNovo, telNovo, emailNovo, &editado);
                printf(editado ? "  Contato atualizado!\n" : "  Erro ao editar.\n");
                break;
            }

            /* -- 6. REMOVER ---------------------------------- */
            case 6: {
                char nome[TAM_NOME], telefone[TAM_TELEFONE];
                printf("  Nome do contato:     "); lerLinha(nome,     sizeof(nome));
                printf("  Telefone do contato: "); lerLinha(telefone, sizeof(telefone));

                /* Verifica se existe antes de pedir confirmacao */
                if (!buscarNo(raiz, nome, telefone)) {
                    printf("  Contato nao encontrado.\n");
                    break;
                }

                /* Confirmacao — operacao destrutiva nao deve ser silenciosa */
                char confirm[4];
                printf("  Tem certeza que deseja remover '%s'? (s/n): ", nome);
                lerLinha(confirm, sizeof(confirm));
                if (confirm[0] != 's' && confirm[0] != 'S') {
                    printf("  Remocao cancelada.\n");
                    break;
                }

                int removido = 0;
                raiz = remover(raiz, nome, telefone, &removido);
                printf(removido ? "  Contato removido!\n" : "  Erro ao remover.\n");
                break;
            }

            /* -- 7. SALVAR ----------------------------------- */
            case 7: {
                salvarDados(raiz, ARQUIVO);
                break;
            }

            /* -- 8. SALVAR E SAIR ---------------------------- */
            case 8: {
                salvarDados(raiz, ARQUIVO);
                liberarArvore(raiz);
                raiz = NULL;
                printf("  Ate logo!\n");
                break;
            }

            default:
                printf("  Opcao invalida. Tente de 1 a 8.\n");
        }

    } while (opcao != 8);

    return 0;
}
