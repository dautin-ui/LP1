#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <windows.h>
#include "TAD_HASH_ARQUIVO.h"

int main()
{
    // configuração UTF-8, para prints com acentos
    setlocale(LC_ALL, ".UTF-8");
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    //------------------------------------------

    hash* h = hsh_cria(1000);
    if (!h)
    {
        printf("Erro ao criar tabela hash.\n");
        return 1;
    }

    int opcao;
    char caminho[256];
    char termo[51];

    printf("Escolha uma opcao:\n");
    printf("1 - Inserir arquivo | 2 - Inserir pasta\n");

    scanf("%d", &opcao);
    getchar(); //remove '\n'

    while (1)
    {
        if (opcao == 1)
        {
            printf("\nDigite o nome ou caminho do arquivo .txt:\n");

            scanf(" %255[^\n]", caminho);
            getchar();

            printf("\nArquivo selecionado:\n%s\n", caminho);

            if (indexar_arquivo(h, caminho))
            break;
        }
        else if (opcao == 2)
        {
            printf("\nDigite o caminho da pasta:\n");

            scanf(" %255[^\n]", caminho);
            getchar();

            printf("\nPasta selecionada:\n%s\n", caminho);

            if(indexar_pasta(h, caminho))
            break;
        }
        else
        {
            printf("\nOpcao invalida! Tente novamente.\n");

            printf("1 - Inserir arquivo\n");
            printf("2 - Inserir pasta\n");

            scanf("%d", &opcao);
            getchar();
        }
    }


    while (1)
    {
        printf("\nPesquise o termo ('0' ou 'sair' para encerrar o programa):\n");

        scanf(" %50[^\n]", termo);
        getchar();

        if (strcmp(termo, "0") == 0 ||
            strcmp(termo, "sair") == 0)
        {
            break;
        }

        // termos menores que 5:
        // busca direta nos arquivos (sem hash)
        if (strlen(termo) < 5)
        {
            printf("\nTermo com menos de 5 caracteres.\n");
            printf("Realizando busca direta...\n\n");

            if (opcao == 1)
            {
                buscar_termo_curto_arquivo(caminho, termo);
            }
            else
            {
                buscar_termo_curto_pasta(caminho, termo);
            }

            continue;
        }

        pesquisar_termo(h, termo);
    }

    hsh_libera(h);

    printf("\nPrograma encerrado.\n");

    return 0;
}