#ifndef GRAFO_H
#define GRAFO_H

#include <stdbool.h>
#include <stdio.h>

/**
 * @file grafo.h
 * @brief Grafo direcionado com listas de adjacência para o sistema viário de Bitnópolis.
 * @details
 * Vértices representam extremos de segmentos de rua (esquinas e meios de quadra).
 * Arestas representam segmentos de rua com sentido de tráfego.
 */

typedef void* Grafo;
typedef void* Vertice;
typedef void* Aresta;


// Grafos


/**
 * @brief Cria um grafo vazio.
 * @param capacidade_vertices Estimativa do número de vértices (para pré-alocação).
 * @return Ponteiro opaco para o grafo, ou NULL em caso de falha.
 */
Grafo criarGrafo(int capacidade_vertices);

/**
 * @brief Libera toda a memória do grafo, incluindo vértices e arestas.
 * @param g O grafo a ser destruído.
 */
void destruirGrafo(Grafo g);


// Vértices


/**
 * @brief Insere um vértice no grafo.
 * @param g  O grafo.
 * @param id Identificador único do vértice (string).
 * @param x Coordenada X do vértice.
 * @param y Coordenada Y do vértice.
 * @return Ponteiro para o vértice criado, ou NULL em caso de falha.
 */
Vertice inserirVertice(Grafo g, const char* id, double x, double y);

/**
 * @brief Busca um vértice pelo seu identificador.
 * @param g O grafo.
 * @param id Identificador do vértice.
 * @return Ponteiro para o vértice encontrado, ou NULL se não existir.
 */
Vertice buscarVertice(Grafo g, const char* id);

/**
 * @brief Retorna o número total de vértices no grafo.
 * @param g O grafo.
 * @return Número de vértices.
 */
int getNumVertices(Grafo g);


// Getters de vértice


/** @brief Retorna o identificador do vértice.
 * @param v O vértice.
 * @return Identificador do vértice.
 */
const char* getVerticeId(Vertice v);

/** @brief Retorna a coordenada X do vértice.
 * @param v O vértice.
 * @return Coordenada X do vértice.
 */
double getVerticeX(Vertice v);

/** @brief Retorna a coordenada Y do vértice.
 * @param v O vértice.
 * @return Coordenada Y do vértice.
 */
double getVerticeY(Vertice v);


// Arestas


/**
 * @brief Insere uma aresta direcionada (i -> j) no grafo.
 * @param g O grafo.
 * @param id_i Identificador do vértice de origem.
 * @param id_j Identificador do vértice de destino.
 * @param nome Nome da rua.
 * @param ldir CEP da quadra à direita do segmento ("-" se ausente).
 * @param lesq CEP da quadra à esquerda do segmento ("-" se ausente).
 * @param cmp Comprimento do segmento em metros.
 * @param vm Velocidade média em m/s.
 * @return Ponteiro para a aresta criada, ou NULL em caso de falha.
 */
Aresta inserirAresta(Grafo g, const char* id_i, const char* id_j, const char* nome, const char* ldir, const char* lesq, double cmp, double vm);

/**
 * @brief Retorna o número total de arestas no grafo.
 * @param g O grafo.
 * @return Número de arestas.
 */
int getNumArestas(Grafo g);


// Getters de aresta


/** @brief Retorna o nome da rua da aresta.
 * @param a A aresta.
 * @return Nome da rua da aresta.
 */
const char* getArestaNome(Aresta a);

/** @brief Retorna o CEP da quadra à direita.
 * @param a A aresta.
 * @return CEP da quadra à direita.
 */
const char* getArestaLdir(Aresta a);

/** @brief Retorna o CEP da quadra à esquerda.
 * @param a A aresta.
 * @return CEP da quadra à esquerda.
 */
const char* getArestaLesq(Aresta a);

/** @brief Retorna o comprimento do segmento em metros.
 * @param a A aresta.
 * @return Comprimento do segmento em metros.
 */
double getArestaCmp(Aresta a);

/** @brief Retorna a velocidade média em m/s.
 * @param a A aresta.
 * @return Velocidade média em m/s.
 */
double getArestaVm(Aresta a);

/** @brief Retorna o vértice de origem da aresta.
 * @param a A aresta.
 * @return Vértice de origem da aresta.
 */
Vertice getArestaOrigem(Aresta a);

/** @brief Retorna o vértice de destino da aresta.
 * @param a A aresta.
 * @return Vértice de destino da aresta.
 */
Vertice getArestaDestino(Aresta a);

/**
 * @brief Atualiza a velocidade média de uma aresta.
 * @param a     A aresta.
 * @param nova_vm Nova velocidade média em m/s.
 */
void setArestaVm(Aresta a, double nova_vm);


// Iteração sobre adjacências


/**
 * @brief Itera sobre todas as arestas que saem de um vértice.
 * @param g O grafo.
 * @param v O vértice de origem.
 * @param contexto Dado extra passado para a função de visita.
 * @param visitar Função chamada para cada aresta saindo de v.
 */
void iterarAdjacentes(Grafo g, Vertice v, void* contexto, void (*visitar)(Aresta aresta, void* contexto));

/**
 * @brief Itera sobre todos os vértices do grafo.
 * @param g O grafo.
 * @param contexto Dado extra passado para a função de visita.
 * @param visitar  Função chamada para cada vértice.
 */
void iterarVertices(Grafo g, void* contexto, void (*visitar)(Vertice v, void* contexto));

/**
 * @brief Itera sobre todas as arestas do grafo.
 * @param g O grafo.
 * @param contexto Dado extra passado para a função de visita.
 * @param visitar Função chamada para cada aresta.
 */
void iterarArestas(Grafo g, void* contexto, void (*visitar)(Aresta a, void* contexto));


// Desenho SVG


/**
 * @brief Desenha todos os vértices e arestas do grafo no SVG.
 * @param g O grafo.
 * @param fSvg Arquivo SVG aberto para escrita.
 */
void desenharGrafoSVG(Grafo g, FILE* fSvg);

#endif