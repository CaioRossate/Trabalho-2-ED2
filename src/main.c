#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "hashing.h"
#include "geo.h"
#include "qry.h"
#include "via.h"
#include "grafo.h"

typedef enum { 
    ENTRADA, SAIDA, GEO, QUERY, VIA 
} FilePaths;

char* concatenarCaminho(const char* dir, const char* arquivo) {
    char* buf = malloc(512);
    if (!dir || strlen(dir) == 0)
        strncpy(buf, arquivo, 511);
    else
        snprintf(buf, 512, "%s/%s", dir, arquivo);
    return buf;
}

int main(int argc, char* argv[]) {

    srand((unsigned int)time(NULL));

    const char* uso = "Uso: ted [-e path] -f arq.geo [-q consulta.qry] [-v arq.via] -o dir\n";

    char* flags[] = {"-e", "-o", "-f", "-q", "-v"};
    char** paths  = calloc(5, sizeof(char*));

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printf("%s", uso);
            free(paths);
            return 0;
        }

        int reconhecida = 0;
        for (int j = 0; j < 5; j++) {
            if (strcmp(argv[i], flags[j]) == 0) {
                reconhecida = 1;
                if (i + 1 < argc) {
                    paths[j] = argv[++i];
                } else {
                    fprintf(stderr, "Erro: flag '%s' requer um argumento.\n", argv[i]);
                }
                break;
            }
        }
        if (!reconhecida)
            fprintf(stderr, "Aviso: argumento desconhecido ignorado: '%s'\n", argv[i]);
    }

    if (!paths[SAIDA] || !paths[GEO]) {
        fprintf(stderr, "%s", uso);
        free(paths);
        return 1;
    }

    // Hash de quadras
    Hash hashQuadras = inicializarHash(5, getQuadraSize());
    if (!hashQuadras) {
        fprintf(stderr, "Erro ao inicializar hash de quadras.\n");
        free(paths);
        return 1;
    }


    char* pathGeo = concatenarCaminho(paths[ENTRADA], paths[GEO]);
    processarArquivoGeo(pathGeo, hashQuadras);

    // Grafo viário (.via)
    Grafo grafo = NULL;
    if (paths[VIA]) {
        char* pathVia = concatenarCaminho(paths[ENTRADA], paths[VIA]);
        grafo = processarArquivoVia(pathVia);
        if (!grafo) fprintf(stderr, "Aviso: nao foi possivel carregar o arquivo .via: %s\n", pathVia);
        free(pathVia);
    }

    // Arquivo .qry
    if (paths[QUERY]) {
        char* pathQry = concatenarCaminho(paths[ENTRADA], paths[QUERY]);

        // Monta nomes de saída
        char geoBase[128], qryBase[128];
        strncpy(geoBase, paths[GEO], sizeof(geoBase)-1); 
        geoBase[sizeof(geoBase)-1] = '\0';
        strncpy(qryBase, paths[QUERY], sizeof(qryBase)-1); 
        qryBase[sizeof(qryBase)-1] = '\0';
        char* d = strrchr(geoBase, '.'); 
        if (d) *d = '\0';
        char* b = strrchr(qryBase, '/'); 
        if (b) memmove(qryBase, b+1, strlen(b));

        char pathTxt[512], pathSvg[512];
        snprintf(pathTxt, sizeof(pathTxt), "%s/%s-%s.txt", paths[SAIDA], geoBase, qryBase);
        snprintf(pathSvg, sizeof(pathSvg), "%s/%s-%s.svg", paths[SAIDA], geoBase, qryBase);

        FILE* fTxt = fopen(pathTxt, "w");
        FILE* fSvg = fopen(pathSvg, "w");
        if (!fTxt) fprintf(stderr, "Erro ao criar TXT: %s\n", pathTxt);
        if (!fSvg) fprintf(stderr, "Erro ao criar SVG: %s\n", pathSvg);

        if (fTxt && fSvg) {
            // Pré-leitura dos CEPs removidos (retorna 0 no T2)
            char ceps_removidos[1][20];
            int  n_rem = coletarCepsRemovidos(pathQry, ceps_removidos, 1);

            // Calcula viewBox a partir das quadras
            double vx, vy, vw, vh;
            calcularBBoxCidade(hashQuadras, &vx, &vy, &vw, &vh);

            // Expande o viewBox para incluir os vértices do grafo
            if (grafo) {
                double gx, gy, gw, gh;
                calcularBBoxGrafo(grafo, &gx, &gy, &gw, &gh);
                // Toma o menor x/y e maior x+w/y+h entre quadras e grafo
                double min_x = vx < gx ? vx : gx;
                double min_y = vy < gy ? vy : gy;
                double max_x = (vx+vw) > (gx+gw) ? (vx+vw) : (gx+gw);
                double max_y = (vy+vh) > (gy+gh) ? (vy+vh) : (gy+gh);
                vx = min_x; vy = min_y;
                vw = max_x - min_x;
                vh = max_y - min_y;
            }

            fprintf(fSvg, "<svg xmlns=\"http://www.w3.org/2000/svg\"" " xmlns:xlink=\"http://www.w3.org/1999/xlink\""
                " viewBox=\"%lf %lf %lf %lf\"" " width=\"100%%\" height=\"100%%\">\n", vx, vy, vw, vh);

            // Desenha quadras
            gerarCidadeSVG(hashQuadras, fSvg, ceps_removidos, n_rem);

            // Desenha grafo viário se disponível
            if (grafo) desenharGrafoSVG(grafo, fSvg);

            // Processa comandos .qry
            processarArquivoQry(pathQry, hashQuadras, grafo, fTxt, fSvg);

            fprintf(fSvg, "</svg>\n");
            fclose(fTxt);
            fclose(fSvg);
        } else {
            if (fTxt) fclose(fTxt);
            if (fSvg) fclose(fSvg);
        }
        free(pathQry);
    }

    
    // Liberação
    encerrarHash(hashQuadras);
    if (grafo) destruirGrafo(grafo);
    free(pathGeo);
    free(paths);

    printf("Processamento concluido com sucesso.\n");
    return 0;
}