#ifndef DIJKSTRA_H
#define DIJKSTRA_H
#include "stdlib.h"

#include "grafo.h"

/**
 * @file dijkstra.h
 * @brief Algoritmo de Dijkstra que serve para encontrar os caminhos mínimos no grafo viário.
 *
 * Dois critérios de otimização são suportados:
 *  -> Distância: minimiza o comprimento total do percurso (metros).
 *  -> Tempo: minimiza o tempo total de percurso (segundos = cmp / vm).
 */

/**
 * @brief Tipo opaco que representa o resultado de uma execução de Dijkstra.
 * @details Armazena os vetores de distância/predecessor para todos os vértices a partir de uma origem.
 */
typedef void* ResultadoDijkstra;

/**
 * @brief Critério de otimização a usar no Dijkstra.
 */
typedef enum {
    DIJKSTRA_DISTANCIA, // Minimiza comprimento total (metros).
    DIJKSTRA_TEMPO // Minimiza tempo total (segundos).
} CriterioDijkstra;

/**
 * @brief Executa o algoritmo de Dijkstra a partir de um vértice de origem.
 * @param g O grafo viário.
 * @param origem Vértice de partida.
 * @param criterio Critério de otimização (distância ou tempo).
 * @return Resultado alocado dinamicamente; liberar com destruirResultado().
 *         Retorna NULL em caso de falha de alocação.
 */
ResultadoDijkstra executarDijkstra(Grafo g, Vertice origem, CriterioDijkstra criterio);

/**
 * @brief Libera a memória do resultado de Dijkstra.
 * @param r O resultado a ser destruído.
 */
void destruirResultado(ResultadoDijkstra r);

/**
 * @brief Retorna o custo mínimo do vértice de origem até o destino.
 * @param r Resultado de Dijkstra.
 * @param destino Vértice de destino.
 * @return Custo mínimo, ou -1.0 se o destino for inacessível.
 */
double getCusto(ResultadoDijkstra r, Vertice destino);

/**
 * @brief Reconstrói o caminho mínimo da origem até o destino.
 * @details O caminho é retornado como um array de Vertice alocado dinamicamente,
 *  em ordem origem → destino. O chamador deve liberar o array com free().
 * @param r Resultado de Dijkstra.
 * @param destino Vértice de destino.
 * @param tamanho Saída: número de vértices no caminho.
 * @return Array de Vertice com o caminho, ou NULL se inacessível.
 */
Vertice* reconstruirCaminho(ResultadoDijkstra r, Vertice destino, int* tamanho);

#endif