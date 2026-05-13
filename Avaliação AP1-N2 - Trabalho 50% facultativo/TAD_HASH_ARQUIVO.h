#ifndef TAD_HASH_ARQUIVO_H
#define TAD_HASH_ARQUIVO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <locale.h>

#ifdef _WIN32
#include <conio.h>
#endif



// lista para guardar ocorrências
typedef struct ocorrencia
{
    char nome_arquivo[256];
    long offset;
    struct ocorrencia* prox;

} ocorrencia;


// registro que guarda um ponteiro para lista de ocorrências e o termo
typedef struct registroIndice
{
    char termo[51];
    ocorrencia* lista;

} registroIndice;


//lista do nó da tabela hash, com registro de ocorrências
typedef struct noHash
{
    registroIndice reg;
    struct noHash* prox;

} noHash;

//tabela hash, que tem tamanho e ponteiro para a lista de nós
typedef struct hash
{
    int tam;
    noHash** tab;

} hash;



// cria uma tabela com tam definido pelo usuário
hash* hsh_cria(int tam);

// função hash
int hsh_funcao(const char* termo, int tam);

// libera memoria apartir de uma tabela enviada como parametro
void hsh_libera(hash* h);

// insere termo na tabela
void hsh_insere(hash* h,const char* termo,const char* nome_arquivo,long offset);

registroIndice* hsh_busca(hash* h, const char* termo);

//pega o arquivo pelo path ou pelo nome
int indexar_arquivo(hash* h, const char* nome_arquivo);

//mesma estrutura da função anterior, porém ela percorre uma pasta
int indexar_pasta(hash* h, const char* caminho_pasta);


//busca o termo apartir de uma tabela e uma palavra digitada pelo usuário
void pesquisar_termo(hash* h, const char* termo);

//mostra palvras "ao redor" da palavra buscada
void mostrar_contexto(const char* nome_arquivo, long offset);


//funções para buscar sem armazenar na tabela hash, sendo essas buscas de casos exclusivos de menos de 5 letras
void buscar_termo_curto_arquivo(const char* nome_arquivo, const char* termo);

void buscar_termo_curto_pasta(const char* pasta,const char* termo);

#endif