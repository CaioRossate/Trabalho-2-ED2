#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "hashing.h"

#define BUCKET_CAPACIDADE 10

// O cabeçalho do bucket (profundidade_local + quantidade) é gravado em disco 
// seguido pelos dados dos registros. O tamanho total do bucket em disco é: sizeof(int)*2  +  BUCKET_CAPACIDADE * tam_reg

typedef struct {
    int profundidade_local;
    int quantidade;
} BucketHeader;

typedef struct {
    FILE*  arquivo;
    int    profundidade_global;
    size_t tam_reg;
    size_t tam_bucket_disco; 
    int    total_indices;
    long*  offsets;
} HashExtensivel;

// Hash DJB2
static uint64_t gerar_hash_djb2(const char* chave) {
    uint64_t hash = 5381;
    int c;
    while ((c = (unsigned char)*chave++)) hash = ((hash << 5) + hash) + c;
    return hash;
}

// I/O de buckets dinâmicos

static void salvar_bucket(HashExtensivel* h, long offset, BucketHeader* hdr, char* dados) {
    fseek(h->arquivo, offset, SEEK_SET);
    fwrite(hdr,  sizeof(BucketHeader), 1, h->arquivo);
    fwrite(dados, h->tam_reg, BUCKET_CAPACIDADE, h->arquivo);
}

// Carrega cabeçalho + dados do offset indicado.'dados'
// deve ter capacidade de BUCKET_CAPACIDADE * tam_reg bytes
static void carregar_bucket(HashExtensivel* h, long offset, BucketHeader* hdr, char* dados) {
    fseek(h->arquivo, offset, SEEK_SET);
    fread(hdr,  sizeof(BucketHeader), 1, h->arquivo);
    fread(dados, h->tam_reg, BUCKET_CAPACIDADE, h->arquivo);
}

// Aloca um novo bucket vazio no final do arquivo e retorna seu offset
static long alocar_bucket(HashExtensivel* h, int profundidade) {
    fseek(h->arquivo, 0, SEEK_END);
    long offset = ftell(h->arquivo);

    BucketHeader hdr = { profundidade, 0 };
    char* zeros = calloc(BUCKET_CAPACIDADE, h->tam_reg);
    fwrite(&hdr,  sizeof(BucketHeader), 1, h->arquivo);
    fwrite(zeros, h->tam_reg, BUCKET_CAPACIDADE, h->arquivo);
    free(zeros);
    return offset;
}


Hash inicializarHash(const char* nome_arquivo, int profundidade_inicial, size_t tamanho_registro) {
    HashExtensivel* h = malloc(sizeof(HashExtensivel));
    if (!h) return NULL;

    h->tam_reg = tamanho_registro;
    h->tam_bucket_disco = sizeof(BucketHeader) + BUCKET_CAPACIDADE * tamanho_registro;
    h->arquivo = fopen(nome_arquivo, "w+b");
    if (!h->arquivo) { free(h); return NULL; }

    h->profundidade_global = profundidade_inicial;
    h->total_indices = 1 << profundidade_inicial;
    h->offsets = malloc(sizeof(long) * h->total_indices);
    if (!h->offsets) { fclose(h->arquivo); free(h); return NULL; }

    long offset_inicial = alocar_bucket(h, profundidade_inicial);
    for (int i = 0; i < h->total_indices; i++)
        h->offsets[i] = offset_inicial;

    return (Hash)h;
}

bool inserirHash(Hash h, void* dado) {
    HashExtensivel* he = (HashExtensivel*)h;
    const char* chave  = (const char*)dado;
    uint64_t v_hash    = gerar_hash_djb2(chave);
    int trava          = 0;

    // buffer reutilizável para dados do bucket
    char* dados = malloc(BUCKET_CAPACIDADE * he->tam_reg);
    if (!dados) return false;

    while (1) {
        if (trava++ > 64) {
            fprintf(stderr, "Erro: loop de split detectado.\n");
            free(dados);
            return false;
        }

        int  idx = (int)(v_hash & (uint64_t)((1 << he->profundidade_global) - 1));
        long offset_alvo = he->offsets[idx];

        BucketHeader hdr;
        carregar_bucket(he, offset_alvo, &hdr, dados);

        for (int i = 0; i < hdr.quantidade; i++) {
            char* reg = dados + i * he->tam_reg;
            if (strcmp(reg, chave) == 0) {
                memcpy(reg, dado, he->tam_reg);
                salvar_bucket(he, offset_alvo, &hdr, dados);
                free(dados);
                return true;
            }
        }

        // Inserção simples se há espaço
        if (hdr.quantidade < BUCKET_CAPACIDADE) {
            memcpy(dados + hdr.quantidade * he->tam_reg, dado, he->tam_reg);
            hdr.quantidade++;
            salvar_bucket(he, offset_alvo, &hdr, dados);
            free(dados);
            return true;
        }

        // Split necessário

        // Expande o diretório se profundidade local == global
        if (hdr.profundidade_local == he->profundidade_global) {
            int tam_antigo = he->total_indices;
            he->profundidade_global++;
            he->total_indices = 1 << he->profundidade_global;
            he->offsets = realloc(he->offsets, sizeof(long) * he->total_indices);
            for (int i = 0; i < tam_antigo; i++)
                he->offsets[i + tam_antigo] = he->offsets[i];
        }

        // Cria novo bucket
        int bit_separador = 1 << hdr.profundidade_local;
        hdr.profundidade_local++;
        long novo_offset = alocar_bucket(he, hdr.profundidade_local);

        // Backup dos registros atuais
        char* backup = malloc(BUCKET_CAPACIDADE * he->tam_reg);
        if (!backup) { free(dados); return false; }
        int qtd_backup = hdr.quantidade;
        memcpy(backup, dados, qtd_backup * he->tam_reg);

        // Limpa bucket original
        hdr.quantidade = 0;
        memset(dados, 0, BUCKET_CAPACIDADE * he->tam_reg);
        salvar_bucket(he, offset_alvo, &hdr, dados);

        // Atualiza diretório: entradas com o bit separador apontam para o novo bucket
        for (int i = 0; i < he->total_indices; i++) {
            if (he->offsets[i] == offset_alvo && (i & bit_separador))
                he->offsets[i] = novo_offset;
        }

        // Redistribui os registros
        for (int i = 0; i < qtd_backup; i++) {
            char* reg  = backup + i * he->tam_reg;
            uint64_t hv = gerar_hash_djb2(reg);
            int idx2   = (int)(hv & (uint64_t)((1 << he->profundidade_global) - 1));
            long odest = he->offsets[idx2];

            BucketHeader hdr2;
            char* dados2 = malloc(BUCKET_CAPACIDADE * he->tam_reg);
            carregar_bucket(he, odest, &hdr2, dados2);
            if (hdr2.quantidade < BUCKET_CAPACIDADE) {
                memcpy(dados2 + hdr2.quantidade * he->tam_reg, reg, he->tam_reg);
                hdr2.quantidade++;
                salvar_bucket(he, odest, &hdr2, dados2);
            }
            free(dados2);
        }
        free(backup);
        // Loop: tenta inserir o dado original novamente
    }
}

bool buscarHash(Hash h, char* chave, void* destino) {
    HashExtensivel* he = (HashExtensivel*)h;
    uint64_t v_hash    = gerar_hash_djb2(chave);
    int idx = (int)(v_hash & (uint64_t)((1 << he->profundidade_global) - 1));

    BucketHeader hdr;
    char* dados = malloc(BUCKET_CAPACIDADE * he->tam_reg);
    carregar_bucket(he, he->offsets[idx], &hdr, dados);

    for (int i = 0; i < hdr.quantidade; i++) {
        char* reg = dados + i * he->tam_reg;
        if (strcmp(reg, chave) == 0) {
            memcpy(destino, reg, he->tam_reg);
            free(dados);
            return true;
        }
    }
    free(dados);
    return false;
}

bool removerHash(Hash h, char* chave) {
    HashExtensivel* he = (HashExtensivel*)h;
    uint64_t v_hash    = gerar_hash_djb2(chave);
    int idx  = (int)(v_hash & (uint64_t)((1 << he->profundidade_global) - 1));
    long off = he->offsets[idx];

    BucketHeader hdr;
    char* dados = malloc(BUCKET_CAPACIDADE * he->tam_reg);
    carregar_bucket(he, off, &hdr, dados);

    for (int i = 0; i < hdr.quantidade; i++) {
        char* reg = dados + i * he->tam_reg;
        if (strcmp(reg, chave) == 0) {
            // Substitui pelo último para não deixar buracos
            if (i < hdr.quantidade - 1)
                memcpy(reg, dados + (hdr.quantidade - 1) * he->tam_reg, he->tam_reg);
            hdr.quantidade--;
            salvar_bucket(he, off, &hdr, dados);
            free(dados);
            return true;
        }
    }
    free(dados);
    return false;
}

void percorrerHash(Hash h, void* contexto, void (*funcao_visita)(void* registro, void* contexto)) {
    HashExtensivel* he = (HashExtensivel*)h;
    long* visitados    = malloc(sizeof(long) * he->total_indices);
    int   qtd_v        = 0;
    char* dados        = malloc(BUCKET_CAPACIDADE * he->tam_reg);

    for (int i = 0; i < he->total_indices; i++) {
        long offset = he->offsets[i];

        bool ja_foi = false;
        for (int j = 0; j < qtd_v; j++)
            if (visitados[j] == offset) { ja_foi = true; break; }
        if (ja_foi) continue;

        BucketHeader hdr;
        carregar_bucket(he, offset, &hdr, dados);
        for (int j = 0; j < hdr.quantidade; j++)
            funcao_visita(dados + j * he->tam_reg, contexto);

        visitados[qtd_v++] = offset;
    }
    free(dados);
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
    fclose(he->arquivo);
    free(he->offsets);
    free(he);
}

void gerarRelatorioHash(Hash h, const char* nome_arquivo_relatorio) {
    HashExtensivel* he = (HashExtensivel*)h;
    FILE* rel = fopen(nome_arquivo_relatorio, "w");
    if (!rel) return;

    fprintf(rel, "--- RELATORIO HASH EXTENSIVEL ---\n");
    fprintf(rel, "Profundidade Global : %d\n", he->profundidade_global);
    fprintf(rel, "Tamanho do Diretorio: %d entradas\n", he->total_indices);
    fprintf(rel, "Tamanho do Registro : %zu bytes\n", he->tam_reg);
    fprintf(rel, "Tamanho do Bucket   : %zu bytes (cabecalho + %d registros)\n\n", he->tam_bucket_disco, BUCKET_CAPACIDADE);

    long* visitados = malloc(sizeof(long) * he->total_indices);
    int   qtd_v     = 0;
    char* dados     = malloc(BUCKET_CAPACIDADE * he->tam_reg);

    for (int i = 0; i < he->total_indices; i++) {
        long offset = he->offsets[i];
        bool ja_foi = false;
        for (int j = 0; j < qtd_v; j++)
            if (visitados[j] == offset) { ja_foi = true; break; }

        fprintf(rel, "Indice [%d] -> offset %ld%s\n", i, offset, ja_foi ? " (ja listado)" : "");

        if (!ja_foi) {
            BucketHeader hdr;
            carregar_bucket(he, offset, &hdr, dados);
            fprintf(rel, "   > Profundidade Local: %d\n", hdr.profundidade_local);
            fprintf(rel, "   > Ocupacao: %d/%d\n", hdr.quantidade, BUCKET_CAPACIDADE);
            visitados[qtd_v++] = offset;
        }
    }

    free(dados);
    free(visitados);
    fclose(rel);
}