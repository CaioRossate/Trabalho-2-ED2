#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include "geo.h"
#include "hashing.h"

typedef struct __attribute__((packed)) {
    char cep[20];
    double x, y, w, h;
    char cor_p[20];
    char cor_b[20];
    char sw[10];
} quadra_t;

// getters
size_t getQuadraSize() {
    return sizeof(quadra_t); 
}
char* getQuadraCEP(Quadra q) {
    return (q) ? ((quadra_t*)q)->cep : NULL; 
}
double getQuadraX(Quadra q) {
    return (q) ? ((quadra_t*)q)->x : 0; 
}
double getQuadraY(Quadra q) {
    return (q) ? ((quadra_t*)q)->y : 0;
}
double getQuadraW(Quadra q) {
    return (q) ? ((quadra_t*)q)->w : 0; 
}
double getQuadraH(Quadra q) {
    return (q) ? ((quadra_t*)q)->h : 0; 
}
const char* getQuadraCorP(Quadra q) {
    return (q) ? ((quadra_t*)q)->cor_p : NULL; 
}
const char* getQuadraCorB(Quadra q) {
    return (q) ? ((quadra_t*)q)->cor_b : NULL; 
}
const char* getQuadraSW(Quadra q) {
    return (q) ? ((quadra_t*)q)->sw : NULL; 
}

// Criar/destruir
Quadra criarQuadra(char* cep, double x, double y, double w, double h, char* cor_b, char* cor_p, char* sw) {
    quadra_t* q = calloc(1, sizeof(quadra_t));
    if (q) {
        strncpy(q->cep, cep, 19);
        strncpy(q->cor_b, cor_b, 19);
        strncpy(q->cor_p, cor_p, 19);
        strncpy(q->sw, sw, 9);
        q->x = x; 
        q->y = y; 
        q->w = w; 
        q->h = h;
    }
    return (Quadra)q;
}

Quadra comando_cq(char* cfill, char* cstrk, char* sw) {
    return criarQuadra("", 0, 0, 0, 0, cstrk, cfill, sw);
}

void destruirQuadra(Quadra q) {
    if (q) free(q); 
}

// leitura .geo
bool processarArquivoGeo(const char* path_geo, Hash hash_quadras) {
    FILE* arquivo = fopen(path_geo, "r");
    if (!arquivo) return false;

    char comando[10];
    char cfill[20] = "white";
    char cstrk[20] = "black";
    char sw[10] = "1.0";

    while (fscanf(arquivo, "%9s", comando) != EOF) {
        if (strcmp(comando, "cq") == 0) {
            fscanf(arquivo, "%9s %19s %19s", sw, cfill, cstrk);
        } else if (strcmp(comando, "q") == 0) {
            char cep[20];
            double x, y, w, h;
            fscanf(arquivo, "%19s %lf %lf %lf %lf", cep, &x, &y, &w, &h);
            Quadra nova = criarQuadra(cep, x, y, w, h, cstrk, cfill, sw);
            inserirHash(hash_quadras, nova);
            destruirQuadra(nova);
        }
    }
    fclose(arquivo);
    return true;
}

// bounding box
typedef struct {
    double min_x, min_y, max_x, max_y; 
} BBox;

static void acumularBBox(void* reg, void* ctx) {
    BBox* bb = (BBox*)ctx;
    double x2 = getQuadraX(reg) + getQuadraW(reg);
    double y2 = getQuadraY(reg) + getQuadraH(reg);
    if (getQuadraX(reg) < bb->min_x) bb->min_x = getQuadraX(reg);
    if (getQuadraY(reg) < bb->min_y) bb->min_y = getQuadraY(reg);
    if (x2 > bb->max_x) bb->max_x = x2;
    if (y2 > bb->max_y) bb->max_y = y2;
}

void calcularBBoxCidade(Hash h_q, double* vx, double* vy, double* vw, double* vh) {
    BBox bb = { DBL_MAX, DBL_MAX, -DBL_MAX, -DBL_MAX };
    percorrerHash(h_q, &bb, acumularBBox);
    double margem = 10.0;
    *vx = bb.min_x - margem;
    *vy = bb.min_y - margem;
    *vw = (bb.max_x - bb.min_x) + 2 * margem;
    *vh = (bb.max_y - bb.min_y) + 2 * margem;
}

// SVG — contexto para desenho com filtro de CEPs removidos 
typedef struct {
    FILE* svg;
    char (*ceps_removidos)[20];
    int n_removidos;
} CtxDesenho;

static int cepFoiRemovido(const char* cep, char ceps[][20], int n) {
    for (int i = 0; i < n; i++)
        if (strcmp(cep, ceps[i]) == 0) return 1;
    return 0;
}

static void desenharQuadraSVG(void* reg, void* ctx) {
    CtxDesenho* c = (CtxDesenho*)ctx;
    if (!reg || !c->svg) return;

    // Pula quadras que foram removidas pelo rq 
    if (cepFoiRemovido(getQuadraCEP(reg), c->ceps_removidos, c->n_removidos))
        return;

    fprintf(c->svg, "\t<rect x=\"%lf\" y=\"%lf\" width=\"%lf\" height=\"%lf\"" " fill=\"%s\" stroke=\"%s\" stroke-width=\"%s\" />\n",
        getQuadraX(reg), getQuadraY(reg), getQuadraW(reg), getQuadraH(reg), getQuadraCorP(reg), getQuadraCorB(reg), getQuadraSW(reg));

    fprintf(c->svg, "\t<text x=\"%lf\" y=\"%lf\" font-size=\"4\" fill=\"black\">%s</text>\n",  getQuadraX(reg) + 2, getQuadraY(reg) + 10, getQuadraCEP(reg));
}

void gerarCidadeSVG(Hash h_q, FILE* fSvg, char ceps_removidos[][20], int n_removidos) {
    if (!h_q || !fSvg) return;
    CtxDesenho ctx = { fSvg, ceps_removidos, n_removidos };
    percorrerHash(h_q, &ctx, desenharQuadraSVG);
}