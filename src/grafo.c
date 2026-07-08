#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grafo.h"
#include "hashing.h"


typedef struct aresta_no {
    struct aresta_t* aresta;
    struct aresta_no* prox;
} aresta_no_t;

typedef struct vertice_t {
    char id[64];
    double x, y;
    aresta_no_t* adj; // lista de adjacências (arestas saindo deste vértice)
    struct vertice_t* prox; // encadeamento na lista de vértices do grafo
    int indice; // índice numérico para Dijkstra
} vertice_t;

typedef struct aresta_t {
    vertice_t* origem;
    vertice_t* destino;
    char nome[64];
    char ldir[32];
    char lesq[32];
    double cmp;
    double vm;
    int ativo; // 1 = ativa, 0 = desativada
    struct aresta_t* prox_global; // encadeamento na lista global de arestas
} aresta_t;

typedef struct {
    vertice_t* vertices; // lista encadeada de vértices
    vertice_t* ultimo_v; // ponteiro para o fim da lista (inserção O(1))
    aresta_t* arestas; // lista global de arestas
    aresta_t* ultima_a;
    int num_vertices;
    int num_arestas;
    Hash indice_vertices; // id -> vertice_t*, para busca 0(1) 
} grafo_t;

typedef struct __attribute__((packed)) {
    char id[64];
    vertice_t* v;
} vertice_idx_t;

// Grafo

Grafo criarGrafo(int capacidade_vertices) {
    (void)capacidade_vertices; // lista encadeada
    grafo_t* g = calloc(1, sizeof(grafo_t));
    if (!g) return NULL;
    g->indice_vertices = inicializarHash(8, sizeof(vertice_idx_t));
    if (!g->indice_vertices) { 
        free(g); 
        return NULL; 
    }
    return (Grafo)g;
}

void destruirGrafo(Grafo g) {
    if (!g) return;
    grafo_t* gr = (grafo_t*)g;

    // Libera listas de adjacência e vértices
    vertice_t* v = gr->vertices;
    while (v) {
        aresta_no_t* no = v->adj;
        while (no) {
            aresta_no_t* tmp = no->prox;
            free(no);
            no = tmp;
        }
        vertice_t* tv = v->prox;
        free(v);
        v = tv;
    }

    // Libera arestas
    aresta_t* a = gr->arestas;
    while (a) {
        aresta_t* ta = a->prox_global;
        free(a);
        a = ta;
    }
    if (gr->indice_vertices) encerrarHash(gr->indice_vertices);

    free(gr);
}


// Vértices

Vertice inserirVertice(Grafo g, const char* id, double x, double y) {
    if (!g || !id) return NULL;
    grafo_t* gr = (grafo_t*)g;

    vertice_t* v = calloc(1, sizeof(vertice_t));
    if (!v) return NULL;

    strncpy(v->id, id, 63);
    v->x = x;
    v->y = y;
    v->adj = NULL;
    v->prox = NULL;
    v->indice = gr->num_vertices;

    // Insere no fim da lista
    if (!gr->vertices) {
        gr->vertices = v;
        gr->ultimo_v = v;
    } else {
        gr->ultimo_v->prox = v;
        gr->ultimo_v = v;
    }
    gr->num_vertices++;

    vertice_idx_t idxreg;
    memset(idxreg.id, 0, sizeof(idxreg.id));
    strncpy(idxreg.id, v->id, sizeof(idxreg.id) - 1);
    idxreg.v = v;
    inserirHash(gr->indice_vertices, &idxreg);

    return (Vertice)v;
}

Vertice buscarVertice(Grafo g, const char* id) {
    if (!g || !id) return NULL;
    grafo_t* gr = (grafo_t*)g;
    vertice_idx_t idxreg;
    if (!buscarHash(gr->indice_vertices, (char*)id, &idxreg)) return NULL;
    return (Vertice)idxreg.v;
}

int getNumVertices(Grafo g) {
    return g ? ((grafo_t*)g)->num_vertices : 0;
}

const char* getVerticeId(Vertice v) {
    return v ? ((vertice_t*)v)->id : NULL;
}

double getVerticeX(Vertice v) {
    return v ? ((vertice_t*)v)->x : 0.0;
}

double getVerticeY(Vertice v) {
    return v ? ((vertice_t*)v)->y : 0.0;
}

// Acesso ao índice usado internamente pelo Dijkstra
int getVerticeIndice(Vertice v) {
    return v ? ((vertice_t*)v)->indice : -1;
}

// Arestas

Aresta inserirAresta(Grafo g, const char* id_i, const char* id_j, const char* nome, const char* ldir, const char* lesq, double cmp, double vm) {
    if (!g) return NULL;
    grafo_t* gr = (grafo_t*)g;

    vertice_t* vi = (vertice_t*)buscarVertice(g, id_i);
    vertice_t* vj = (vertice_t*)buscarVertice(g, id_j);
    if (!vi || !vj) {
        fprintf(stderr, "inserirAresta: vertice '%s' ou '%s' nao encontrado.\n", id_i, id_j);
        return NULL;
    }

    aresta_t* a = calloc(1, sizeof(aresta_t));
    if (!a) return NULL;

    a->origem  = vi;
    a->destino = vj;
    strncpy(a->nome, nome, 63);
    strncpy(a->ldir, ldir, 31);
    strncpy(a->lesq, lesq, 31);
    a->cmp = cmp;
    a->vm  = vm;
    a->ativo = 1; // por padrão, todas as arestas são criadas ativas
    
    // Adiciona na lista de adjacência do vértice de origem
    aresta_no_t* no = malloc(sizeof(aresta_no_t));
    if (!no) { free(a); return NULL; }
    no->aresta = a;
    no->prox = vi->adj;
    vi->adj = no;

    // Adiciona na lista global de arestas
    if (!gr->arestas) {
        gr->arestas = a;
        gr->ultima_a = a;
    } else {
        gr->ultima_a->prox_global = a;
        gr->ultima_a = a;
    }
    gr->num_arestas++;
    return (Aresta)a;
}

int getNumArestas(Grafo g) {
    return g ? ((grafo_t*)g)->num_arestas : 0;
}

const char* getArestaNome(Aresta a) {
    return a ? ((aresta_t*)a)->nome : NULL;
}
const char* getArestaLdir(Aresta a) {
    return a ? ((aresta_t*)a)->ldir : NULL;
}
const char* getArestaLesq(Aresta a) {
    return a ? ((aresta_t*)a)->lesq : NULL;
}
double getArestaCmp(Aresta a) {
    return a ? ((aresta_t*)a)->cmp : 0.0;
}
double getArestaVm(Aresta a) {
    return a ? ((aresta_t*)a)->vm : 0.0;
}
Vertice getArestaOrigem(Aresta a) {
    return a ? ((aresta_t*)a)->origem : NULL;
}
Vertice getArestaDestino(Aresta a){
    return a ? ((aresta_t*)a)->destino : NULL;
}

void setArestaVm(Aresta a, double nova_vm) {
    if (a) ((aresta_t*)a)->vm = nova_vm;
}

bool getArestaAtiva(Aresta a) {
    return a ? ((aresta_t*)a)->ativo : false;
}

void desabilitarAresta(Aresta a) {
    if (a) ((aresta_t*)a)->ativo = 0;
}

void habilitarAresta(Aresta a) {
    if (a) ((aresta_t*)a)->ativo = 1;
}

void habilitarTodasArestas(Grafo g) {
    if (!g) return;
    aresta_t* a = ((grafo_t*)g)->arestas;
    while (a) {
        a->ativo = 1;
        a = a->prox_global;
    }
}

// Iteradores

void iterarAdjacentes(Grafo g, Vertice v, void* contexto, void (*visitar)(Aresta, void*)) {
    if (!g || !v || !visitar) return;
    aresta_no_t* no = ((vertice_t*)v)->adj;
    while (no) {
        // Só visita arestas ativas
        if (no->aresta->ativo)
            visitar((Aresta)no->aresta, contexto);
        no = no->prox;
    }
}

int* getVizinhosAtivos(Grafo g, Vertice v, int* n_vizinhos) {
    *n_vizinhos = 0;
    if (!g || !v) return NULL;
    // Conta primeiro
    int cap = 0;
    aresta_no_t* no = ((vertice_t*)v)->adj;
    while (no) { 
        if (no->aresta->ativo) cap++; 
        no = no->prox; 
    }
    if (cap == 0) return NULL;
    int* buf = malloc(sizeof(int) * cap);
    no = ((vertice_t*)v)->adj;
    while (no) {
        if (no->aresta->ativo)
            buf[(*n_vizinhos)++] = no->aresta->destino->indice;
        no = no->prox;
    }
    return buf;
}

void iterarVertices(Grafo g, void* contexto, void (*visitar)(Vertice, void*)) {
    if (!g || !visitar) return;
    vertice_t* v = ((grafo_t*)g)->vertices;
    while (v) {
        visitar((Vertice)v, contexto);
        v = v->prox;
    }
}

void iterarArestas(Grafo g, void* contexto, void (*visitar)(Aresta, void*)) {
    if (!g || !visitar) return;
    aresta_t* a = ((grafo_t*)g)->arestas;
    while (a) {
        if (a->ativo)
        visitar((Aresta)a, contexto);
        a = a->prox_global;
    }
}


// SVG

static void desenharVertice(Vertice v, void* ctx) {
    FILE* svg = (FILE*)ctx;
    fprintf(svg, "\t<circle cx=\"%lf\" cy=\"%lf\" r=\"3\""" fill=\"orange\" stroke=\"black\" stroke-width=\"0.5\"/>\n", getVerticeX(v), getVerticeY(v));
    fprintf(svg, "\t<text x=\"%lf\" y=\"%lf\" font-size=\"4\" fill=\"black\">%s</text>\n", getVerticeX(v) + 4, getVerticeY(v) - 2, getVerticeId(v));
}

static void desenharAresta(Aresta a, void* ctx) {
    FILE* svg = (FILE*)ctx;
    double x1 = getVerticeX(getArestaOrigem(a));
    double y1 = getVerticeY(getArestaOrigem(a));
    double x2 = getVerticeX(getArestaDestino(a));
    double y2 = getVerticeY(getArestaDestino(a));

    fprintf(svg, "\t<line x1=\"%lf\" y1=\"%lf\" x2=\"%lf\" y2=\"%lf\""" stroke=\"gray\" stroke-width=\"1\""" marker-end=\"url(#seta)\"/>\n", x1, y1, x2, y2);
}

void calcularBBoxGrafo(Grafo g, double* vx, double* vy, double* vw, double* vh) {
    if (!g) { 
        *vx = *vy = *vw = *vh = 0; 
        return; 
    }
    double min_x = 1e18, min_y = 1e18;
    double max_x = -1e18, max_y = -1e18;
    vertice_t* v = ((grafo_t*)g)->vertices;
    while (v) {
        if (v->x < min_x) min_x = v->x;
        if (v->y < min_y) min_y = v->y;
        if (v->x > max_x) max_x = v->x;
        if (v->y > max_y) max_y = v->y;
        v = v->prox;
    }
    double margem = 20.0;
    *vx = min_x - margem;
    *vy = min_y - margem;
    *vw = (max_x - min_x) + 2 * margem;
    *vh = (max_y - min_y) + 2 * margem;
}

void desenharGrafoSVG(Grafo g, FILE* fSvg) {
    if (!g || !fSvg) return;

    // Define marcador de seta 
    fprintf(fSvg, "\t<defs>\n""\t  <marker id=\"seta\" markerWidth=\"6\" markerHeight=\"6\""" refX=\"6\" refY=\"3\" orient=\"auto\">\n""\t <path d=\"M0,0 L0,6 L6,3 z\" fill=\"gray\"/>\n""\t </marker>\n""\t</defs>\n");

    iterarArestas(g, fSvg, desenharAresta);
    iterarVertices(g, fSvg, desenharVertice);
}