#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "TAD_HASH_ARQUIVO.h"

// protótipo interno
static void normalizar(char* palavra)
{
    int i, j = 0;

    // converte para minúsculo + remove acentos
    for (i = 0; palavra[i]; i++)
    {
        unsigned char c = (unsigned char)palavra[i];
        c = tolower(c);

        palavra[j++] = (char)c;
    }

    palavra[j] = '\0';

    // remove lixo do final
    while (j > 0 && !isalnum((unsigned char)palavra[j - 1]))
    {
        palavra[--j] = '\0';
    }

    // remove lixo do início
    int inicio = 0;

    while (palavra[inicio] && !isalnum((unsigned char)palavra[inicio]))
    {
        inicio++;
    }

    if (inicio > 0)
    {
        memmove(palavra, palavra + inicio,strlen(palavra + inicio) + 1);
    }
}


hash* hsh_cria(int tam)
{
    if (tam <= 0)
        return NULL;

    hash* h = (hash*)malloc(sizeof(hash));

    if (!h)
        return NULL;

    h->tam = tam;

    h->tab = (noHash**)calloc(tam, sizeof(noHash*));

    if (!h->tab)
    {
        free(h);
        return NULL;
    }

    return h;
}

int hsh_funcao(const char* termo, int tam)
{
    unsigned long valor = 5381;
    int i = 0;

    while (termo[i] != '\0')
    {
        valor = valor * 31 + (unsigned char)termo[i];
        i++;
    }

    return (int)(valor % tam);
}

registroIndice* hsh_busca(hash* h, const char* termo)
{
    if (!h || !termo)
        return NULL;

    int indice = hsh_funcao(termo, h->tam);

    noHash* p = h->tab[indice];

    while (p)
    {
        if (strcmp(p->reg.termo, termo) == 0) return &p->reg;

        p = p->prox;
    }

    return NULL;
}

void hsh_insere(hash* h, const char* termo, const char* nome_arquivo, long offset)
{
    if (!h || !termo || !nome_arquivo)
        return;

    registroIndice* r = hsh_busca(h, termo);

    // termo já existe
    if (r)
    {
        ocorrencia* verif = r->lista;

        while (verif)
        {
            if (strcmp(verif->nome_arquivo, nome_arquivo) == 0 && verif->offset == offset)
            {
                return;
            }

            verif = verif->prox;
        }

        ocorrencia* nova = (ocorrencia*)malloc(sizeof(ocorrencia));

        if (!nova)
            return;

        strncpy(nova->nome_arquivo,nome_arquivo,sizeof(nova->nome_arquivo) - 1);

        nova->nome_arquivo[sizeof(nova->nome_arquivo) - 1] = '\0';

        nova->offset = offset;

        nova->prox = r->lista;

        r->lista = nova;

        return;
    }

    // novo termo
    int indice = hsh_funcao(termo, h->tam);

    noHash* novo = (noHash*)malloc(sizeof(noHash));

    if (!novo)return;

    strncpy(novo->reg.termo,termo,sizeof(novo->reg.termo) - 1);

    novo->reg.termo[sizeof(novo->reg.termo) - 1] = '\0';

    ocorrencia* oc =(ocorrencia*)malloc(sizeof(ocorrencia));

    if (!oc)
    {
        free(novo);
        return;
    }

    strncpy(oc->nome_arquivo,nome_arquivo,sizeof(oc->nome_arquivo) - 1);

    oc->nome_arquivo[sizeof(oc->nome_arquivo) - 1] = '\0';

    oc->offset = offset;

    oc->prox = NULL;

    novo->reg.lista = oc;

    novo->prox = h->tab[indice];

    h->tab[indice] = novo;
}

void hsh_libera(hash* h)
{
    if (!h)
        return;

    for (int i = 0; i < h->tam; i++)
    {
        noHash* p = h->tab[i];

        while (p)
        {
            ocorrencia* oc = p->reg.lista;

            while (oc)
            {
                ocorrencia* prox_oc = oc->prox;

                free(oc);

                oc = prox_oc;
            }

            noHash* prox = p->prox;

            free(p);

            p = prox;
        }
    }

    free(h->tab);
    free(h);
}

//auxiliares

static int eh_separador(int c)
{
    return (c == ' '  ||
            c == '\t' ||
            c == '\n' ||
            c == '\r');
}

int esperar_enter_ou_esc()
{
#ifdef _WIN32

    int c = _getch();

    if (c == 27)
        return 0;

    return 1;

#else

    getchar();
    return 1;

#endif
}



int indexar_arquivo(hash* h,const char* nome_arquivo)
{
    if (!h || !nome_arquivo)
        return 0;

    FILE* fp = fopen(nome_arquivo, "rb");

    if (!fp)
    {
        printf("Nao foi possivel abrir: %s\n", nome_arquivo);

        return 0;
    }

    char palavra[51];
    int c, k;
    long offset;

    while (1)
    {
        while ((c = fgetc(fp)) != EOF && eh_separador(c));

        if (c == EOF)
            break;

        offset = ftell(fp) - 1;

        k = 0;

        palavra[k++] = (char)c;

        while (k < 50 && (c = fgetc(fp)) != EOF && !eh_separador(c))
        {
            palavra[k++] = (char)c;
        }

        palavra[k] = '\0';

        normalizar(palavra);

        // apenas palavras >= 5 entram na hash
        if (strlen(palavra) >= 5)
        {
            hsh_insere(h,palavra,nome_arquivo, offset);
        }
    }

    fclose(fp);

    return 1;
}

int indexar_pasta(hash* h,const char* caminho_pasta)
{
    if (!h || !caminho_pasta)
        return 0;

    DIR* dir = opendir(caminho_pasta);

    if (!dir)
    {
        printf("Nao foi possivel abrir a pasta.\n");
        return 0;
    }

    struct dirent* entrada;
    char caminho_completo[512];

    while ((entrada = readdir(dir)) != NULL)
    {
        if (strcmp(entrada->d_name, ".") == 0 || strcmp(entrada->d_name, "..") == 0)
        {
            continue;
        }

        const char* ext = strrchr(entrada->d_name, '.');

        if (!ext || strcmp(ext, ".txt") != 0)
            continue;

        snprintf(caminho_completo,sizeof(caminho_completo),"%s/%s",caminho_pasta,entrada->d_name);

        indexar_arquivo(h,caminho_completo);
    }

    closedir(dir);

    return 1;
}


void mostrar_contexto(const char* nome_arquivo,long offset)
{
    FILE* fp = fopen(nome_arquivo, "rb");

    if (!fp)
        return;

    char trecho[101];
    long inicio = offset - 40;

   while (inicio > 0)
{
    fseek(fp, inicio, SEEK_SET);

    unsigned char b = fgetc(fp);

    // byte que NÃO é continuação UTF-8
    if ((b & 0xC0) != 0x80)
        break;

    inicio++;
}

    int lidos =
        fread(trecho, 1, 50, fp);

    trecho[lidos] = '\0';

    for (int i = 0; trecho[i]; i++)
    {
        if (trecho[i] == '\n' || trecho[i] == '\r')
        {
            trecho[i] = ' ';
        }
    }

    printf("...%s...\n", trecho);

    fclose(fp);
}

//busca de termos menores que 5 letras

void buscar_termo_curto_arquivo(
    const char* nome_arquivo,
    const char* termo)
{
    FILE* fp = fopen(nome_arquivo, "rb");

    if (!fp)
        return;

    char linha[1024];

    while (fgets(linha,sizeof(linha),fp))
    {
        char copia[1024];

        strncpy(copia, linha, sizeof(copia) - 1);

        copia[sizeof(copia) - 1] = '\0';

        normalizar(copia);

        if (strstr(copia, termo))
        {
            long offset = ftell(fp) - strlen(linha);

            printf("%s - ",nome_arquivo);

            mostrar_contexto(nome_arquivo,offset);

            printf("\n");

            printf("Enter para continuar ou ESC para parar.\n");

            if (!esperar_enter_ou_esc())
            {
                fclose(fp);
                return;
            }
        }
    }

    fclose(fp);
}

void buscar_termo_curto_pasta(const char* pasta, const char* termo)
{
    DIR* dir = opendir(pasta);

    if (!dir) return;

    struct dirent* entrada;

    char caminho[512];

    while ((entrada = readdir(dir)) != NULL)
    {
        if (strcmp(entrada->d_name, ".") == 0 || strcmp(entrada->d_name, "..") == 0)
            continue;

        const char* ext = strrchr(entrada->d_name, '.');

        if (!ext || strcmp(ext, ".txt") != 0)
            continue;

        snprintf(caminho,sizeof(caminho),"%s/%s", pasta, entrada->d_name);

        buscar_termo_curto_arquivo(caminho, termo);
    }

    closedir(dir);
}


void pesquisar_termo(hash* h, const char* termo)
{
    if (!h || !termo)
        return;

    char termo_normalizado[51];

    strncpy(termo_normalizado, termo, sizeof(termo_normalizado) - 1);

    termo_normalizado[sizeof(termo_normalizado) - 1] = '\0';

    normalizar(termo_normalizado);
    registroIndice* r = hsh_busca(h,termo_normalizado);

    if (!r)
    {
        printf("Termo \"%s\" nao encontrado.\n",termo);
        return;
    }

    int total = 0;

    ocorrencia* oc = r->lista;

    while (oc)
    {
        total++;

        oc = oc->prox;
    }

    printf("\nTermo \"%s\" - %d ocorrencia(s)\n", termo_normalizado, total);
    printf("---------------------------------\n");

    oc = r->lista;
    int i = 1;
    int contador = 0;

    while (oc)
    {
        printf("[%d] %s\n", i, oc->nome_arquivo);

        printf("    ");

        mostrar_contexto(oc->nome_arquivo, oc->offset);

        printf("\n");

        contador++;

        if (contador % 5 == 0)
        {
            printf("Enter para continuar ou ESC para parar.\n");
            if (!esperar_enter_ou_esc())
                break;
        }

        oc = oc->prox;

        i++;
    }
}