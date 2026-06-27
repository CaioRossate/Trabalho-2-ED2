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

    char* flags[] = {"-e", "-o", "-f", "-q", "-v"};
    char** paths  = calloc(5, sizeof(char*));

    for (int i = 1; i < argc; i += 2)
        for (int j = 0; j < 5; j++)
            if (i + 1 < argc && strcmp(argv[i], flags[j]) == 0)
                paths[j] = argv[i + 1];

    if (!paths[SAIDA] || !paths[GEO]) {
        printf("Uso: ted [-e path] -f arq.geo [-q consulta.qry] [-v arq.via] -o dir\n");
        free(paths);
        return 1;
    }

    // Hash de quadras
    Hash hashQuadras = inicializarHash("quadras.hf", 5, getQuadraSize());
    if (!hashQuadras) {
        printf("Erro ao inicializar hash de quadras.\n");
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
        if (!grafo) printf("Aviso: nao foi possivel carregar o arquivo .via: %s\n", pathVia);
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
        if (!fTxt) printf("Erro ao criar TXT: %s\n", pathTxt);
        if (!fSvg) printf("Erro ao criar SVG: %s\n", pathSvg);

        if (fTxt && fSvg) {
            // Pré-leitura dos CEPs removidos (retorna 0 no T2)
            char ceps_removidos[1][20];
            int  n_rem = coletarCepsRemovidos(pathQry, ceps_removidos, 1);

            // Abre SVG com viewBox calculado
            double vx, vy, vw, vh;
            calcularBBoxCidade(hashQuadras, &vx, &vy, &vw, &vh);
            fprintf(fSvg, "<svg xmlns=\"http://www.w3.org/2000/svg\"" " xmlns:xlink=\"http://www.w3.org/1999/xlink\""
                " viewBox=\"%lf %lf %lf %lf\"" " width=\"100%%\" height=\"100%%\">\n",vx, vy, vw, vh);

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

    // Relatório do hash
    gerarRelatorioHash(hashQuadras, "relatorio_quadras.hfd");

    // Liberação
    encerrarHash(hashQuadras);
    if (grafo) destruirGrafo(grafo);
    free(pathGeo);
    free(paths);

    printf("Processamento concluido com sucesso.\n");
    return 0;
}