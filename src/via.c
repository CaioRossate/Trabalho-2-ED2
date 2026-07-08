#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "via.h"
#include "grafo.h"

Grafo processarArquivoVia(const char* path_via) {
    FILE* arq = fopen(path_via, "r");
    if (!arq) return NULL;

    char linha[1024];
    int num_linha = 0;

    if (!fgets(linha, sizeof(linha), arq)) { 
        fclose(arq); 
        return NULL; 
    }
    num_linha++;
    int num_vertices = 0;
    if (sscanf(linha, "%d", &num_vertices) != 1) {
        fprintf(stderr, "via.c: linha %d invalida (esperado numero de vertices): %s", num_linha, linha);
        fclose(arq);
        return NULL;
    }

    Grafo g = criarGrafo(num_vertices);
    if (!g) {
        fclose(arq);
        return NULL;
    }

    while (fgets(linha, sizeof(linha), arq)) {
        num_linha++;
        char tipo[4] = "";
        if (sscanf(linha, "%3s", tipo) != 1) continue; // linha vazia

        if (strcmp(tipo, "v") == 0) {
            char id[64];
            double x, y;
            if (sscanf(linha, "%*s %63s %lf %lf", id, &x, &y) == 3) {
                inserirVertice(g, id, x, y);
            } else {
                fprintf(stderr, "via.c: linha %d malformada (esperado 'v id x y'): %s", num_linha, linha);
            }

        } else if (strcmp(tipo, "e") == 0) {
            char id_i[64], id_j[64], ldir[32], lesq[32], nome[128];
            double cmp, vm;
            if (sscanf(linha, "%*s %63s %63s %31s %31s %lf %lf %127s",
                       id_i, id_j, ldir, lesq, &cmp, &vm, nome) == 7) {
                // inserirAresta avisa no stderr se vertice nao existir
                inserirAresta(g, id_i, id_j, nome, ldir, lesq, cmp, vm);
            } else {
                fprintf(stderr, "via.c: linha %d malformada (esperado 'e i j ldir lesq cmp vm nome'): %s", num_linha, linha);
            }
        }
        // outros tipos/linhas em branco sao ignorados silenciosamente
    }
    fclose(arq);
    return g;
}
