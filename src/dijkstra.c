#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include "dijkstra.h"
#include "grafo.h"

typedef struct {
    int n;
    double* custo;
    int* pred; // pred[i] = índice do predecessor, -1 se nenhum
    Vertice* idx_para_v; // mapa índice -> Vertice 
    Vertice origem;
} resultado_t;

// Array de heap para o Dijkstra (custo/índice)

typedef struct {
    double custo;
    int idx; 
} HeapNo;

typedef struct {
    HeapNo* dados;
    int tamanho;
    int capacidade;
} Heap;

static Heap* criarHeap(int cap) {
    Heap* h = malloc(sizeof(Heap));
    h->dados = malloc(sizeof(HeapNo) * cap);
    h->tamanho = 0;
    h->capacidade = cap;
    return h;
}

static void destruirHeap(Heap* h) {
    if (h) {
        free(h->dados);
        free(h); 
    }
}

static void trocar(HeapNo* a, HeapNo* b) {
    HeapNo t = *a;
    *a = *b; 
    *b = t;
}

static void subir(Heap* h, int i) {
    while (i > 0) {
        int pai = (i - 1) / 2;
        if (h->dados[pai].custo > h->dados[i].custo) {
            trocar(&h->dados[pai], &h->dados[i]);
            i = pai;
        } else break;
    }
}

static void descer(Heap* h, int i) {
    while (1) {
        int menor = i, e = 2*i+1, d = 2*i+2;
        if (e < h->tamanho && h->dados[e].custo < h->dados[menor].custo) menor = e;
        if (d < h->tamanho && h->dados[d].custo < h->dados[menor].custo) menor = d;
        if (menor == i) break;
        trocar(&h->dados[i], &h->dados[menor]);
        i = menor;
    }
}

static void inserirHeap(Heap* h, double custo, int idx) {
    if (h->tamanho == h->capacidade) {
        h->capacidade *= 2;
        h->dados = realloc(h->dados, sizeof(HeapNo) * h->capacidade);
    }
    h->dados[h->tamanho].custo = custo;
    h->dados[h->tamanho].idx = idx;
    subir(h, h->tamanho++);
}

static HeapNo extrairMin(Heap* h) {
    HeapNo min = h->dados[0];
    h->dados[0] = h->dados[--h->tamanho];
    if (h->tamanho > 0) descer(h, 0);
    return min;
}

// Parte para montar mapa índice -> Vertice

typedef struct { 
    Vertice* mapa; 
    int pos; 
} CtxMapa;

static void mapearVertice(Vertice v, void* ctx) {
    CtxMapa* c = (CtxMapa*)ctx;
    c->mapa[c->pos++] = v;
}

// Parte arestas no Dijkstra

typedef struct {
    resultado_t* r;
    CriterioDijkstra criterio;
    Heap* heap;
    int iu;
    double cu;
} CtxRelax;

static void relaxarAresta(Aresta a, void* ctx) {
    CtxRelax* c = (CtxRelax*)ctx;
    Vertice dest = getArestaDestino(a);
    int id = getVerticeIndice(dest);
    double vm = getArestaVm(a);
    double peso = (c->criterio == DIJKSTRA_DISTANCIA) ? getArestaCmp(a) : (vm > 0.0 ? getArestaCmp(a) / vm : DBL_MAX);
    double novo = c->cu + peso;
    if (novo < c->r->custo[id]) {
        c->r->custo[id] = novo;
        c->r->pred[id] = c->iu;
        inserirHeap(c->heap, novo, id);
    }
}

// Dijkstra

ResultadoDijkstra executarDijkstra(Grafo g, Vertice origem, CriterioDijkstra criterio) {
    if (!g || !origem) return NULL;
    int n = getNumVertices(g);
    if (n == 0) return NULL;

    resultado_t* r = malloc(sizeof(resultado_t));
    r->n = n;
    r->origem = origem;
    r->custo = malloc(sizeof(double) * n);
    r->pred = malloc(sizeof(int) * n);
    r->idx_para_v = malloc(sizeof(Vertice) * n);

    for (int i = 0; i < n; i++) {
        r->custo[i] = DBL_MAX;
        r->pred[i] = -1;
    }

    // Monta mapa índice -> Vertice
    CtxMapa cm = { r->idx_para_v, 0 };
    iterarVertices(g, &cm, mapearVertice);

    int io = getVerticeIndice(origem);
    r->custo[io] = 0.0;

    Heap* heap = criarHeap(n + 1);
    inserirHeap(heap, 0.0, io);

    while (heap->tamanho > 0) {
        HeapNo no = extrairMin(heap);
        int iu = no.idx;
        if (no.custo > r->custo[iu]) continue;

        Vertice u = r->idx_para_v[iu];
        CtxRelax cr = { r, criterio, heap, iu, r->custo[iu] };
        iterarAdjacentes(g, u, &cr, relaxarAresta);
    }

    destruirHeap(heap);
    return (ResultadoDijkstra)r;
}

void destruirResultado(ResultadoDijkstra r) {
    if (!r) return;
    resultado_t* res = (resultado_t*)r;
    free(res->custo);
    free(res->pred);
    free(res->idx_para_v);
    free(res);
}

double getCusto(ResultadoDijkstra r, Vertice destino) {
    if (!r || !destino) return -1.0;
    resultado_t* res = (resultado_t*)r;
    int idx = getVerticeIndice(destino);
    if (idx < 0 || idx >= res->n) return -1.0;
    return (res->custo[idx] == DBL_MAX) ? -1.0 : res->custo[idx];
}

Vertice* reconstruirCaminho(ResultadoDijkstra r, Vertice destino, int* tamanho) {
    *tamanho = 0;
    if (!r || !destino) return NULL;

    resultado_t* res = (resultado_t*)r;
    int idx_dest = getVerticeIndice(destino);
    int idx_orig = getVerticeIndice(res->origem);

    // Destino inacessível 
    if (res->custo[idx_dest] == DBL_MAX) return NULL;

    // Caso especial: origem == destino
    if (idx_dest == idx_orig) {
        Vertice* cam = malloc(sizeof(Vertice));
        cam[0] = res->origem;
        *tamanho = 1;
        return cam;
    }

    // Usa pilha (tamanho máximo n) para acumular o caminho de trás para frente.
    int* pilha = malloc(sizeof(int) * res->n);
    int topo = 0;
    int cur = idx_dest;

    while (cur != idx_orig) {
        if (cur == -1 || topo >= res->n) {
            // Caminho incompleto: não deveria acontecer se custo != DBL_MAX
            free(pilha);
            return NULL;
        }
        pilha[topo++] = cur;
        cur = res->pred[cur];
    }
    pilha[topo++] = idx_orig; // inclui a origem

    // Inverte a pilha para ordem origem -> destino
    Vertice* caminho = malloc(sizeof(Vertice) * topo);
    for (int i = 0; i < topo; i++)
        caminho[i] = res->idx_para_v[pilha[topo - 1 - i]];

    free(pilha);
    *tamanho = topo;
    return caminho;
}