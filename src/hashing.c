#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "hashing.h"

#define BUCKET_CAPACIDADE 10

typedef struct {
    int profundidade_local;
    int quantidade;
    char* dados; // buffer de BUCKET_CAPACIDADE * tam_reg bytes
} Bucket;

typedef struct {
    int profundidade_global;
    size_t tam_reg;
    int total_indices;
    Bucket** diretorio; // total_indices ponteiros; varias entradas podem apontar para o mesmo bucket
} HashExtensivel;

// Hash DJB2
static uint64_t gerar_hash_djb2(const char* chave) {
    uint64_t hash = 5381;
    int c;
    while ((c = (unsigned char)*chave++)) hash = ((hash << 5) + hash) + c;
    return hash;
}

static Bucket* alocar_bucket(HashExtensivel* h, int profundidade) {
    Bucket* b = malloc(sizeof(Bucket));
    b->profundidade_local = profundidade;
    b->quantidade = 0;
    b->dados = calloc(BUCKET_CAPACIDADE, h->tam_reg);
    return b;
}

Hash inicializarHash(int profundidade_inicial, size_t tamanho_registro) {
    HashExtensivel* h = malloc(sizeof(HashExtensivel));
    if (!h) return NULL;

    h->tam_reg = tamanho_registro;
    h->profundidade_global = profundidade_inicial;
    h->total_indices = 1 << profundidade_inicial;
    h->diretorio = malloc(sizeof(Bucket*) * h->total_indices);
    if (!h->diretorio) { 
        free(h); 
        return NULL; 
    }

    Bucket* inicial = alocar_bucket(h, profundidade_inicial);
    for (int i = 0; i < h->total_indices; i++)
        h->diretorio[i] = inicial;

    return (Hash)h;
}

bool inserirHash(Hash h, void* dado) {
    HashExtensivel* he = (HashExtensivel*)h;
    const char* chave = (const char*)dado;
    uint64_t v_hash = gerar_hash_djb2(chave);
    int trava = 0;

    while (1) {
        if (trava++ > 64) {
            fprintf(stderr, "Erro: loop de split detectado.\n");
            return false;
        }

        int idx = (int)(v_hash & (uint64_t)((1 << he->profundidade_global) - 1));
        Bucket* b = he->diretorio[idx];

        for (int i = 0; i < b->quantidade; i++) {
            char* reg = b->dados + i * he->tam_reg;
            if (strcmp(reg, chave) == 0) {
                memcpy(reg, dado, he->tam_reg);
                return true;
            }
        }

        if (b->quantidade < BUCKET_CAPACIDADE) {
            memcpy(b->dados + b->quantidade * he->tam_reg, dado, he->tam_reg);
            b->quantidade++;
            return true;
        }

        // Split necessário

        if (b->profundidade_local == he->profundidade_global) {
            int tam_antigo = he->total_indices;
            he->profundidade_global++;
            he->total_indices = 1 << he->profundidade_global;
            he->diretorio = realloc(he->diretorio, sizeof(Bucket*) * he->total_indices);
            for (int i = 0; i < tam_antigo; i++)
                he->diretorio[i + tam_antigo] = he->diretorio[i];
        }

        int bit_separador = 1 << b->profundidade_local;
        b->profundidade_local++;
        Bucket* novo = alocar_bucket(he, b->profundidade_local);

        int qtd_backup = b->quantidade;
        char* backup = malloc(BUCKET_CAPACIDADE * he->tam_reg);
        memcpy(backup, b->dados, qtd_backup * he->tam_reg);

        b->quantidade = 0;
        memset(b->dados, 0, BUCKET_CAPACIDADE * he->tam_reg);

        for (int i = 0; i < he->total_indices; i++)
            if (he->diretorio[i] == b && (i & bit_separador))
                he->diretorio[i] = novo;

        for (int i = 0; i < qtd_backup; i++) {
            char* reg = backup + i * he->tam_reg;
            uint64_t hv = gerar_hash_djb2(reg);
            int idx2 = (int)(hv & (uint64_t)((1 << he->profundidade_global) - 1));
            Bucket* destb = he->diretorio[idx2];
            if (destb->quantidade < BUCKET_CAPACIDADE) {
                memcpy(destb->dados + destb->quantidade * he->tam_reg, reg, he->tam_reg);
                destb->quantidade++;
            }
        }
        free(backup);
        // Loop: tenta inserir o dado original novamente
    }
}

bool buscarHash(Hash h, char* chave, void* destino) {
    HashExtensivel* he = (HashExtensivel*)h;
    uint64_t v_hash = gerar_hash_djb2(chave);
    int idx = (int)(v_hash & (uint64_t)((1 << he->profundidade_global) - 1));
    Bucket* b = he->diretorio[idx];

    for (int i = 0; i < b->quantidade; i++) {
        char* reg = b->dados + i * he->tam_reg;
        if (strcmp(reg, chave) == 0) {
            memcpy(destino, reg, he->tam_reg);
            return true;
        }
    }
    return false;
}

bool removerHash(Hash h, char* chave) {
    HashExtensivel* he = (HashExtensivel*)h;
    uint64_t v_hash = gerar_hash_djb2(chave);
    int idx = (int)(v_hash & (uint64_t)((1 << he->profundidade_global) - 1));
    Bucket* b = he->diretorio[idx];

    for (int i = 0; i < b->quantidade; i++) {
        char* reg = b->dados + i * he->tam_reg;
        if (strcmp(reg, chave) == 0) {
            if (i < b->quantidade - 1)
                memcpy(reg, b->dados + (b->quantidade - 1) * he->tam_reg, he->tam_reg);
            b->quantidade--;
            return true;
        }
    }
    return false;
}

void percorrerHash(Hash h, void* contexto, void (*funcao_visita)(void* registro, void* contexto)) {
    HashExtensivel* he = (HashExtensivel*)h;
    Bucket** visitados = malloc(sizeof(Bucket*) * he->total_indices);
    int qtd_v = 0;

    for (int i = 0; i < he->total_indices; i++) {
        Bucket* b = he->diretorio[i];

        bool ja_foi = false;
        for (int j = 0; j < qtd_v; j++)
            if (visitados[j] == b) { 
                ja_foi = true; 
                break; 
            }
        if (ja_foi) continue;

        for (int j = 0; j < b->quantidade; j++)
            funcao_visita(b->dados + j * he->tam_reg, contexto);

        visitados[qtd_v++] = b;
    }
    free(visitados);
}

int getProfundidadeGlobal(Hash h) {
    return ((HashExtensivel*)h)->profundidade_global;
}

int getQuantidadeBuckets(Hash h) {
    return ((HashExtensivel*)h)->total_indices;
}

void encerrarHash(Hash h) {
    if (!h) return;
    HashExtensivel* he = (HashExtensivel*)h;

    Bucket** visitados = malloc(sizeof(Bucket*) * he->total_indices);
    int qtd_v = 0;
    for (int i = 0; i < he->total_indices; i++) {
        Bucket* b = he->diretorio[i];
        bool ja_foi = false;
        for (int j = 0; j < qtd_v; j++)
            if (visitados[j] == b) { 
                ja_foi = true; 
                break; 
            }
        if (!ja_foi) {
            visitados[qtd_v++] = b;
            free(b->dados);
            free(b);
        }
    }
    free(visitados);
    free(he->diretorio);
    free(he);
}
