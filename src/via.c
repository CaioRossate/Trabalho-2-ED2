#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "via.h"
#include "grafo.h"

Grafo processarArquivoVia(const char* path_via) {
    FILE* arq = fopen(path_via, "r");
    if (!arq) return NULL;

    int num_vertices = 0;
    if (fscanf(arq, "%d", &num_vertices) != 1) {
        fclose(arq);
        return NULL;
    }

    Grafo g = criarGrafo(num_vertices);
    if (!g) { 
        fclose(arq); 
        return NULL; 
    }

    char tipo[4];
    while (fscanf(arq, "%3s", tipo) != EOF) {

        if (strcmp(tipo, "v") == 0) {
            char   id[64];
            double x, y;
            if (fscanf(arq, "%63s %lf %lf", id, &x, &y) == 3) inserirVertice(g, id, x, y);

        } else if (strcmp(tipo, "e") == 0) {
            char   id_i[64], id_j[64];
            char   ldir[32], lesq[32];
            double cmp, vm;
            char   nome[128];

            if (fscanf(arq, "%63s %63s %31s %31s %lf %lf %127s", id_i, id_j, ldir, lesq, &cmp, &vm, nome) == 7) {
                // inserirAresta no stderr se vertice nao existir
                inserirAresta(g, id_i, id_j, nome, ldir, lesq, cmp, vm);
            }
        } else {
            char lixo[256];
            fgets(lixo, sizeof(lixo), arq);
        }
    }
    fclose(arq);
    return g;
}