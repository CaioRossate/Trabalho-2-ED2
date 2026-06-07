#ifndef VIA_H
#define VIA_H

#include <stdbool.h>
#include "grafo.h"

/**
 * @file via.h
 * @brief Leitura e construção do grafo viário a partir do arquivo .via.
 *
 * @details
 * Formato do arquivo .via:
 * Primeira linha: número de vértices (inteiro)
 * Linhas seguintes:
 *   v id x y  (define um vértice)
 *   e i j ldir lesq cmp vm nome  (define uma aresta direcionada)
 */

/**
 * @brief Lê o arquivo .via e constrói o grafo viário.
 * @details Os vértices são sempre especificados antes das arestas no arquivo.
 * Caso uma aresta referencie um vértice inexistente, ela é ignorada com aviso no stderr.
 * @param path_via Caminho completo para o arquivo .via.
 * @return Ponteiro (Grafo) populado, ou NULL se o arquivo não puder ser aberto.
 */
Grafo processarArquivoVia(const char* path_via);

#endif