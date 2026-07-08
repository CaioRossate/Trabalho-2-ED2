#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include "qry.h"
#include "grafo.h"
#include "dijkstra.h"
#include "hashing.h"
#include "geo.h"

#define MAX_REG 11

// Registradores geográficos: vértice mais próximo ao endereço 
static Vertice registradores[MAX_REG];

// Utilitários

// Distância euclidiana entre dois pontos
static double distEuclid(double x1, double y1, double x2, double y2) {
    double dx = x2-x1, dy = y2-y1;
    return sqrt(dx*dx + dy*dy);
}

// Calcula coordenada de um endereço cep/face/num usando o hash de quadras
static void coordEndereco(Hash h_q, char* cep, char face, double num, double* ex, double* ey) {
    void* qbuf = malloc(getQuadraSize());
    *ex = 0.0; 
    *ey = 0.0;
    if (!buscarHash(h_q, cep, qbuf)) { 
        free(qbuf); 
        return; 
    }

    double x = getQuadraX(qbuf), y = getQuadraY(qbuf);
    double w = getQuadraW(qbuf), h = getQuadraH(qbuf);

    if (face == 'N') { 
        *ex = x + num; 
        *ey = y + h;     }
    else if (face == 'S') { 
        *ex = x + num; 
        *ey = y; 
    }
    else if (face == 'L') { 
        *ex = x;
        *ey = y + num; 
    }
    else if (face == 'O') { 
        *ex = x + w; 
        *ey = y + num; 
    } else {
        fprintf(stderr, "\n Erro: face invalida '%c' para cep %s\n", face, cep);

    }


    free(qbuf);
}

// Contexto para busca do vértice mais próximo usando ldir/lesq
typedef struct {
    char cep[32];
    double px, py;
    double menor;
    Vertice mais_proximo;
} CtxProximo;

static void verificarArestaProx(Aresta a, void* ctx) {
    CtxProximo* c = (CtxProximo*)ctx;
    // Só considera arestas adjacentes à quadra do endereço
    if (strcmp(getArestaLdir(a), c->cep) != 0 &&
        strcmp(getArestaLesq(a), c->cep) != 0) return;

    Vertice vo = getArestaOrigem(a);
    Vertice vd = getArestaDestino(a);
    double do_ = distEuclid(c->px, c->py, getVerticeX(vo), getVerticeY(vo));
    double dd = distEuclid(c->px, c->py, getVerticeX(vd), getVerticeY(vd));
    if (do_ < c->menor) { 
        c->menor = do_; 
        c->mais_proximo = vo; 
    }
    if (dd < c->menor) { 
        c->menor = dd;  
        c->mais_proximo = vd; 
    }
}

// Fallback: busca euclidiana em todos os vértices (caso o CEP não seja encontrado)
static void verificarVerticeProx(Vertice v, void* ctx) {
    CtxProximo* c = (CtxProximo*)ctx;
    double d = distEuclid(c->px, c->py, getVerticeX(v), getVerticeY(v));
    if (d < c->menor) { 
        c->menor = d; 
        c->mais_proximo = v; 
    }
}

static Vertice verticeProximoEndereco(Grafo grafo, Hash h_q, char* cep, char face, double num) {
    double ex, ey;
    coordEndereco(h_q, cep, face, num, &ex, &ey);

    CtxProximo ctx;
    strncpy(ctx.cep, cep, 31); ctx.cep[31] = '\0';
    ctx.px = ex; 
    ctx.py = ey;
    ctx.menor = DBL_MAX; 
    ctx.mais_proximo = NULL;

    // Tenta primeiro pelas arestas que têm o CEP como ldir ou lesq
    iterarArestas(grafo, &ctx, verificarArestaProx);

    // Se não encontrou nenhuma aresta com o CEP, usa busca global
    if (!ctx.mais_proximo)
        iterarVertices(grafo, &ctx, verificarVerticeProx);

    return ctx.mais_proximo;
}

// Pré-leitura: sem rq no T2, retorna 0

int coletarCepsRemovidos(const char* path_qry, char ceps_out[][20], int max_ceps) {
    (void)path_qry; 
    (void)ceps_out; 
    (void)max_ceps;
    return 0;
}

// Processa Qry

void processarArquivoQry(const char* path_qry, Hash h_q, void* grafo, FILE* fTxt, FILE* fSvg) {
    FILE* arq = fopen(path_qry, "r");
    if (!arq) { 
        fprintf(stderr, "Erro ao abrir .qry: %s\n", path_qry); 
        return; 
    }

    for (int i = 0; i < MAX_REG; i++) registradores[i] = NULL;

    char cmd[16];
    while (fscanf(arq, "%15s", cmd) != EOF) {

        if (strcmp(cmd, "@o?") == 0) {
            char reg_str[8], cep[32], face_str[4]; 
            double num;
            if (fscanf(arq, "%7s %31s %3s %lf", reg_str, cep, face_str, &num) != 4) continue;
            int reg = (reg_str[0]=='R'||reg_str[0]=='r') ? atoi(reg_str+1) : atoi(reg_str);
            if (reg >= 0 && reg < MAX_REG) {
                registradores[reg] = verticeProximoEndereco(grafo, h_q, cep, face_str[0], num);
                double ex, ey;
                coordEndereco(h_q, cep, face_str[0], num, &ex, &ey);
                comando_o(grafo, reg, ex, ey, fTxt, fSvg);
            }

        } else if (strcmp(cmd, "mvm") == 0) {
            double v, x, y, w, h;
            if (fscanf(arq, "%lf %lf %lf %lf %lf", &v, &x, &y, &w, &h) != 5) continue;
            comando_mvm(grafo, v, x, y, w, h);

        } else if (strcmp(cmd, "regs") == 0) {
            double vl;
            if (fscanf(arq, "%lf", &vl) != 1) continue;
            comando_regs(grafo, vl, fTxt, fSvg);

        } else if (strcmp(cmd, "exp") == 0) {
            double vl;
            if (fscanf(arq, "%lf", &vl) != 1) continue;
            comando_exp(grafo, vl, fSvg);

        } else if (strcmp(cmd, "p?") == 0) { 
            char r1_str[8], r2_str[8], cc[32], cr[32];
            if (fscanf(arq, "%7s %7s %31s %31s", r1_str, r2_str, cc, cr) != 4) continue;
            int reg1 = (r1_str[0]=='R'||r1_str[0]=='r') ? atoi(r1_str+1) : atoi(r1_str);
            int reg2 = (r2_str[0]=='R'||r2_str[0]=='r') ? atoi(r2_str+1) : atoi(r2_str);
            comando_p(grafo, reg1, reg2, cc, cr, fTxt, fSvg);

        } else {
            char lixo[256];
            fgets(lixo, sizeof(lixo), arq);
        }
    }
    fclose(arq);
}

// @o? — registrador geográfico

void comando_o(void* grafo, int reg, double ex, double ey, FILE* fTxt, FILE* fSvg) {
    (void)grafo;
    if (reg < 0 || reg >= MAX_REG) return;

    Vertice v = registradores[reg];
    if (!v) {
        fprintf(fTxt, "@o? R%d: nenhum vertice encontrado para o endereco.\n", reg);
        return;
    }

    fprintf(fTxt, "@o? R%d: coord=(%.2lf,%.2lf) vertice=%s (%.2lf,%.2lf)\n", reg, ex, ey, getVerticeId(v), getVerticeX(v), getVerticeY(v));

    // Linha vertical pontilhada vermelha na coordenada X do endereço
    fprintf(fSvg, "\t<line x1=\"%lf\" y1=\"0\" x2=\"%lf\" y2=\"9999\""
        " stroke=\"red\" stroke-width=\"1\" stroke-dasharray=\"5,3\"/>\n", ex, ex);
    // Número do registrador no topo
    fprintf(fSvg, "\t<text x=\"%lf\" y=\"12\" fill=\"red\" font-size=\"10\"" " text-anchor=\"middle\">R%d</text>\n", ex, reg);
}

// mvm — atualiza velocidade média numa região

typedef struct { 
    double v, x, y, w, h; 
} CtxMvm;

static void atualizarVmRegiao(Aresta a, void* ctx) {
    CtxMvm* c = (CtxMvm*)ctx;
    double ox = getVerticeX(getArestaOrigem(a));
    double oy = getVerticeY(getArestaOrigem(a));
    double dx = getVerticeX(getArestaDestino(a));
    double dy = getVerticeY(getArestaDestino(a));
 
    // Atualiza se o ponto MÉDIO da aresta estiver dentro da região
    double mx = (ox + dx) / 2.0;
    double my = (oy + dy) / 2.0;
    if (mx >= c->x && mx <= c->x + c->w && my >= c->y && my <= c->y + c->h)
        setArestaVm(a, c->v);
}

void comando_mvm(void* grafo, double v, double x, double y, double w, double h) {
    CtxMvm ctx = { v, x, y, w, h };
    iterarArestas(grafo, &ctx, atualizarVmRegiao);
}

// regs — CFCs com Tarjan

// Struct para montar mapa índice -> Vertice
typedef struct { 
    Vertice* mapa; 
    int pos; 
} CtxMapaV;

static void mapearV(Vertice v, void* ctx) {
    CtxMapaV* c = (CtxMapaV*)ctx;
    c->mapa[c->pos++] = v;
}
 
// Desabilita arestas rápidas (vm >= vl)
typedef struct { 
    double vl; 
} CtxDesabilitar;

static void desabilitarRapidas(Aresta a, void* ctx) {
    CtxDesabilitar* c = (CtxDesabilitar*)ctx;
    if (getArestaVm(a) < c->vl) desabilitarAresta(a);
}
 
static void gerarCorAleatoria(char* buffer) {
    sprintf(buffer, "#%06X", rand() % 0xFFFFFF);
}
 
// Estado do Tarjan por vértice
typedef struct {
    int disc; // tempo de descoberta (-1 = não visitado)
    int low; // menor disc alcançável 
    int na_pilha; // 1 se está na pilha do Tarjan 
    int componente; // id do CFC (-1 = ainda não atribuído)
} TarjanV;
 
typedef struct {
    Grafo grafo;
    TarjanV* tv;
    int* pilha; // pilha de índices
    int topo,timer, n_comp;
    Vertice* idx_v;
    int n;
} CtxTarjan;
 
// DFS iterativa do Tarjan
static void tarjan_dfs(CtxTarjan* ctx, int inicio) {
    // Pilha de trabalho: (índice, ponteiro para vizinhos, pos_vizinho)
    // array de structs para simular a recursão                  */
    typedef struct { 
        int idx; 
        int* viz; 
        int n_viz, i_viz; 
    } Frame;

    Frame* frames = malloc(sizeof(Frame) * ctx->n);
    int topo_f = 0;
 
    // Empilha o frame inicial 
    Frame f0;
    f0.idx = inicio;
    f0.viz = getVizinhosAtivos(ctx->grafo, ctx->idx_v[inicio], &f0.n_viz);
    f0.i_viz = 0;
    ctx->tv[inicio].disc = ctx->timer;
    ctx->tv[inicio].low = ctx->timer;
    ctx->tv[inicio].na_pilha = 1;
    ctx->timer++;
    ctx->pilha[ctx->topo++] = inicio;
    frames[topo_f++] = f0;
 
    while (topo_f > 0) {
        Frame* cur = &frames[topo_f - 1];
 
        if (cur->i_viz < cur->n_viz) {
            int w = cur->viz[cur->i_viz++];
 
            if (ctx->tv[w].disc == -1) {
                // Vértice não visitado — empilha novo frame
                Frame fn;
                fn.idx = w;
                fn.viz = getVizinhosAtivos(ctx->grafo, ctx->idx_v[w], &fn.n_viz);
                fn.i_viz = 0;
                ctx->tv[w].disc = ctx->timer;
                ctx->tv[w].low = ctx->timer;
                ctx->tv[w].na_pilha = 1;
                ctx->timer++;
                ctx->pilha[ctx->topo++] = w;
                frames[topo_f++] = fn;
            } else if (ctx->tv[w].na_pilha) {
                // Back edge — atualiza low
                if (ctx->tv[w].disc < ctx->tv[cur->idx].low)
                    ctx->tv[cur->idx].low = ctx->tv[w].disc;
            }
        } else {
            // Terminou de explorar cur->idx
            free(cur->viz);
            topo_f--;
 
            if (topo_f > 0) {
                Frame* pai = &frames[topo_f - 1];
                // Propaga low para o pai
                if (ctx->tv[cur->idx].low < ctx->tv[pai->idx].low)
                    ctx->tv[pai->idx].low = ctx->tv[cur->idx].low;
            }
 
            // Verifica se cur->idx é raiz de um CFC
            if (ctx->tv[cur->idx].low == ctx->tv[cur->idx].disc) {
                int comp_id = ctx->n_comp++;
                int w;
                do {
                    w = ctx->pilha[--ctx->topo];
                    ctx->tv[w].na_pilha  = 0;
                    ctx->tv[w].componente = comp_id;
                } while (w != cur->idx);
            }
        }
    }
    free(frames);
}
 
void comando_regs(void* grafo, double vl, FILE* fTxt, FILE* fSvg) {
    int n = getNumVertices(grafo);
 
    // 1. Desabilita arestas rápidas
    habilitarTodasArestas(grafo);
    CtxDesabilitar ctx_des = { vl };
    iterarArestas(grafo, &ctx_des, desabilitarRapidas);
 
    // 2. Monta mapa índice -> Vertice
    Vertice* idx_v = malloc(sizeof(Vertice) * n);
    CtxMapaV cm = { idx_v, 0 };
    iterarVertices(grafo, &cm, mapearV);
 
    // 3. Inicializa estado do Tarjan
    TarjanV* tv = malloc(sizeof(TarjanV) * n);
    for (int i = 0; i < n; i++) {
        tv[i].disc = -1;
        tv[i].low = -1;
        tv[i].na_pilha = 0;
        tv[i].componente = -1;
    }
 
    int* pilha_tarjan = malloc(sizeof(int) * n);
    CtxTarjan ctx_t = { grafo, tv, pilha_tarjan, 0, 0, 0, idx_v, n };
 
    // 4. Roda Tarjan em todos os vértices não visitados
    for (int i = 0; i < n; i++)
        if (tv[i].disc == -1) tarjan_dfs(&ctx_t, i);
 
    // 5. Reabilita todas as arestas
    habilitarTodasArestas(grafo);
 
    int n_comp = ctx_t.n_comp;
    fprintf(fTxt, "regs %.2lf: %d componente(s) fortemente conexo(s)\n", vl, n_comp);
 
    // 6. Bounding boxes por componente
    double* mn_x = malloc(sizeof(double) * n_comp);
    double* mn_y = malloc(sizeof(double) * n_comp);
    double* mx_x = malloc(sizeof(double) * n_comp);
    double* mx_y = malloc(sizeof(double) * n_comp);
    for (int i = 0; i < n_comp; i++) {
        mn_x[i] = 1e18; mn_y[i] = 1e18;
        mx_x[i] = -1e18; mx_y[i] = -1e18;
    }
    for (int i = 0; i < n; i++) {
        int c = tv[i].componente;
        if (c < 0) continue;
        double x = getVerticeX(idx_v[i]);
        double y = getVerticeY(idx_v[i]);
        if (x < mn_x[c]) mn_x[c] = x;
        if (y < mn_y[c]) mn_y[c] = y;
        if (x > mx_x[c]) mx_x[c] = x;
        if (y > mx_y[c]) mx_y[c] = y;
    }
 
    double margem = 10.0;
    for (int i = 0; i < n_comp; i++) {
        // Pula componentes de vértice único (tamanho pontual)
        if (mn_x[i] > 1e17) continue;
        char cor[10];
        gerarCorAleatoria(cor);
        
        fprintf(fSvg,
            "\t<rect x=\"%lf\" y=\"%lf\" width=\"%lf\" height=\"%lf\"" " fill=\"%s\" fill-opacity=\"0.5\" stroke=\"gray\" stroke-width=\"1\"/>\n",
            mn_x[i]-margem, mn_y[i]-margem, (mx_x[i]-mn_x[i])+2*margem, (mx_y[i]-mn_y[i])+2*margem, cor);
    }
 
    free(mn_x); 
    free(mn_y); 
    free(mx_x); 
    free(mx_y);
    free(tv); 
    free(pilha_tarjan); 
    free(idx_v);
}

// exp — AGM (Kruskal) + aumenta vm 50% nas arestas lentas

typedef struct {
    int* visitado;
    double* menor_peso;
    Aresta* melhor;
    int* invertida;  // 1 se a aresta foi usada no sentido inverso (não-direcionado)
} CtxPrim;

typedef struct { 
    int* pai; 
    int n; 
} UF;
 
static UF* uf_criar(int n) {
    UF* uf = malloc(sizeof(UF));
    uf->pai = malloc(sizeof(int) * n);
    uf->n = n;
    for (int i = 0; i < n; i++) uf->pai[i] = i;
    return uf;
}
 
static void uf_destruir(UF* uf) { 
    free(uf->pai); 
    free(uf); 
}
 
static int uf_find_k(UF* uf, int x) {
    while (uf->pai[x] != x) {
        uf->pai[x] = uf->pai[uf->pai[x]];
        x = uf->pai[x];
    }
    return x;
}
 
static int uf_unir_k(UF* uf, int a, int b) {
    a = uf_find_k(uf, a); 
    b = uf_find_k(uf, b);
    if (a == b) return 0;
    uf->pai[b] = a;
    return 1;
}
 
// Array de arestas para o Kruskal
typedef struct { 
    Aresta a; 
    double cmp; 
    int io, id; 
} KEdge;
 
typedef struct { 
    KEdge* arr; 
    int n, cap; 
} CtxKruskal;
 
static void coletarArestaKruskal(Aresta a, void* ctx) {
    CtxKruskal* c = (CtxKruskal*)ctx;
    if (c->n >= c->cap) return;
    c->arr[c->n].a = a;
    c->arr[c->n].cmp = getArestaCmp(a);
    c->arr[c->n].io = getVerticeIndice(getArestaOrigem(a));
    c->arr[c->n].id = getVerticeIndice(getArestaDestino(a));
    c->n++;
}
 
static int cmpKEdge(const void* a, const void* b) {
    double da = ((KEdge*)a)->cmp, db = ((KEdge*)b)->cmp;
    return (da > db) - (da < db);
}
 
void comando_exp(void* grafo, double vl, FILE* fSvg) {
    int n = getNumVertices(grafo);
    int m = getNumArestas(grafo);
 
    // Coleta todas as arestas
    KEdge* arr = malloc(sizeof(KEdge) * m);
    CtxKruskal ctx = { arr, 0, m };
    iterarArestas(grafo, &ctx, coletarArestaKruskal);
 
    // Ordena por comprimento crescente (Kruskal)
    qsort(arr, ctx.n, sizeof(KEdge), cmpKEdge);
 
    // Kruskal: constrói AGM e pinta lentas de vermelho
    UF* uf = uf_criar(n);
    for (int i = 0; i < ctx.n; i++) {
        if (uf_unir_k(uf, arr[i].io, arr[i].id)) {
            // Aresta entrou na AGM — verifica se é lenta
            if (getArestaVm(arr[i].a) < vl) {
                setArestaVm(arr[i].a, getArestaVm(arr[i].a) * 1.5);
                fprintf(fSvg, "\t<line x1=\"%lf\" y1=\"%lf\" x2=\"%lf\" y2=\"%lf\"" " stroke=\"red\" stroke-width=\"4\"/>\n",
                    getVerticeX(getArestaOrigem(arr[i].a)), getVerticeY(getArestaOrigem(arr[i].a)),
                    getVerticeX(getArestaDestino(arr[i].a)), getVerticeY(getArestaDestino(arr[i].a)));
            }
        }
    }
 
    uf_destruir(uf);
    free(arr);
}

// p? — caminhos mínimos animados

static void desenharPercurso(Vertice* cam, int tam, const char* cor, const char* id_path, FILE* fSvg) {
    if (!cam || tam < 2) return;
    fprintf(fSvg, "\t<path id=\"%s\" d=\"M%lf,%lf", id_path, getVerticeX(cam[0]), getVerticeY(cam[0]));
    for (int i = 1; i < tam; i++)
        fprintf(fSvg, " L%lf,%lf", getVerticeX(cam[i]), getVerticeY(cam[i]));
    fprintf(fSvg, "\" stroke=\"%s\" stroke-width=\"3\" fill=\"none\"/>\n", cor);

    fprintf(fSvg, "\t<text font-size=\"50\">\n" "\t <animateMotion dur=\"4s\" repeatCount=\"indefinite\" rotate=\"auto-reverse\">\n"
    "\t <mpath xlink:href=\"#%s\"/>\n" "\t </animateMotion>\n" "\t 🚗\n" "\t</text>\n", id_path);
}

static void desenharPlaca(double x, double y, const char* letra, FILE* fSvg) {
    fprintf(fSvg, "\t<rect x=\"%lf\" y=\"%lf\" width=\"14\" height=\"14\""
        " fill=\"white\" stroke=\"black\" stroke-width=\"1\"/>\n", x-7, y-7);
    fprintf(fSvg, "\t<text x=\"%lf\" y=\"%lf\" font-size=\"10\" text-anchor=\"middle\""
        " dominant-baseline=\"middle\" font-weight=\"bold\">%s</text>\n", x, y, letra);
}

void comando_p(void* grafo, int reg1, int reg2, char* cc, char* cr, FILE* fTxt, FILE* fSvg) {
    if (reg1 < 0 || reg1 >= MAX_REG || reg2 < 0 || reg2 >= MAX_REG) return;

    Vertice origem = registradores[reg1];
    Vertice destino = registradores[reg2];

    if (!origem || !destino) {
        fprintf(fTxt, "p? R%d R%d: registrador invalido.\n", reg1, reg2);
        return;
    }

    // Dijkstra por distância
    ResultadoDijkstra rd = executarDijkstra(grafo, origem, DIJKSTRA_DISTANCIA);
    int tam_d = 0;
    Vertice* cam_d = reconstruirCaminho(rd, destino, &tam_d);

    // Dijkstra por tempo
    ResultadoDijkstra rt = executarDijkstra(grafo, origem, DIJKSTRA_TEMPO);
    int tam_t = 0;
    Vertice* cam_t = reconstruirCaminho(rt, destino, &tam_t);

    if (!cam_d && !cam_t) {
        fprintf(fTxt, "p? R%d R%d: destino inacessivel.\n", reg1, reg2);
    } else {
        // TXT — mais curto
        if (cam_d) {
            fprintf(fTxt, "p? R%d->R%d (mais curto, %.2lf m): ", reg1, reg2, getCusto(rd, destino));
            for (int i = 0; i < tam_d; i++)
                fprintf(fTxt, "%s%s", getVerticeId(cam_d[i]), i<tam_d-1?" -> ":"");
            fprintf(fTxt, "\n\n");
            fprintf(fTxt, " Origem : %s\n", getVerticeId(cam_d[0]));
            fprintf(fTxt, " Destino: %s\n", getVerticeId(cam_d[tam_d-1]));
            fprintf(fTxt, " Percurso: %d segmentos\n\n", tam_d - 1);
        } else {
            fprintf(fTxt, "p? R%d->R%d (mais curto): destino inacessivel por distancia.\n\n", reg1, reg2);
        }

        // TXT — mais rápido
        if (cam_t) {
            fprintf(fTxt, "p? R%d->R%d (mais rapido, %.2lf s): ", reg1, reg2, getCusto(rt, destino));
            for (int i = 0; i < tam_t; i++)
                fprintf(fTxt, "%s%s", getVerticeId(cam_t[i]), i<tam_t-1?" -> ":"");
            fprintf(fTxt, "\n\n");
            fprintf(fTxt, " Origem : %s\n", getVerticeId(cam_t[0]));
            fprintf(fTxt, " Destino: %s\n", getVerticeId(cam_t[tam_t-1]));
            fprintf(fTxt, " Percurso: %d segmentos\n\n", tam_t - 1);
        } else {
            fprintf(fTxt, "p? R%d->R%d (mais rapido): destino inacessivel por tempo.\n\n", reg1, reg2);
        }

        // SVG — percursos animados (só desenha o que existir)
        static int id_seq = 0;
        char id_d[32], id_t[32];
        snprintf(id_d, sizeof(id_d), "path_d_%d", id_seq);
        snprintf(id_t, sizeof(id_t), "path_t_%d", id_seq++);

        if (cam_d) desenharPercurso(cam_d, tam_d, cc, id_d, fSvg);
        if (cam_t) desenharPercurso(cam_t, tam_t, cr, id_t, fSvg);

        // Placas I e F
        desenharPlaca(getVerticeX(origem),  getVerticeY(origem),  "I", fSvg);
        desenharPlaca(getVerticeX(destino), getVerticeY(destino), "F", fSvg);
    }

    free(cam_d);
    free(cam_t);
    destruirResultado(rd);
    destruirResultado(rt);
}
