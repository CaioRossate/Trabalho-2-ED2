#ifndef GEO_H
#define GEO_H

#include <stdbool.h>
#include <stdio.h>
#include "hashing.h"

/**
 * @file geo.h
 * @brief Gerenciamento de quadras e processamento do arquivo .geo.
 */

typedef void* Quadra;

/**
 * @brief Processa o arquivo .geo e popula o hash de quadras.
 * @param path_geo Caminho para o arquivo .geo.
 * @param hash_quadras Hash de quadras a ser populado.
 * @return true se processou corretamente, false caso contrário.
 */
bool processarArquivoGeo(const char* path_geo, Hash hash_quadras);

/**
 * @brief Calcula o bounding box de todas as quadras do hash.
 * @param h_q O manipulador do Hashfile de quadras.
 * @param vx  Saída: X mínimo com margem.
 * @param vy  Saída: Y mínimo com margem.
 * @param vw  Saída: largura total com margem.
 * @param vh  Saída: altura total com margem.
 */
void calcularBBoxCidade(Hash h_q, double* vx, double* vy, double* vw, double* vh);

/**
 * @brief Desenha as quadras no SVG, excluindo as de CEPs removidos.
 * @param h_q Hashfile de quadras.
 * @param fSvg Arquivo SVG aberto para escrita.
 * @param ceps_removidos Array de CEPs a omitir (pode ser NULL).
 * @param n_removidos Quantidade de CEPs no array.
 */
void gerarCidadeSVG(Hash h_q, FILE* fSvg, char ceps_removidos[][20], int n_removidos);

/**
 * @brief Cria uma instância de quadra em memória.
 * @param cep CEP da quadra (chave).
 * @param x Coordenada X.
 * @param y Coordenada Y.
 * @param w Largura.
 * @param h Altura.
 * @param cor_b Cor da borda.
 * @param cor_p Cor de preenchimento.
 * @param sw Espessura da borda.
 * @return Ponteiro (Quadra) ou NULL em caso de falha.
 */
Quadra criarQuadra(char* cep, double x, double y, double w, double h, char* cor_b, char* cor_p, char* sw);

/**
 * @brief Retorna uma quadra-template com cores/espessura fornecidas (comando cq).
 * @param cfill Cor de preenchimento.
 * @param cstrk Cor da borda.
 * @param sw Espessura da borda.
 * @return Ponteiro (Quadra) ou NULL em caso de falha.
 */
Quadra comando_cq(char* cfill, char* cstrk, char* sw);

/**
 * @brief Libera a memória alocada para a quadra.
 * @param q Quadra a ser destruída.
 */
void destruirQuadra(Quadra q);

// Getters

/** 
 * @brief Retorna o tamanho em bytes da struct quadra. 
 */
size_t getQuadraSize();

/** 
 * @brief Retorna o CEP da quadra. 
 * @return Retorna NULL se q for NULL. 
 */
char* getQuadraCEP(Quadra q);

/** 
 * @brief Retorna a coordenada X. 
 * @return Retorna 0 se q for NULL. 
 */
double getQuadraX(Quadra q);

/** 
 * @brief Retorna a coordenada Y. 
 * @return Retorna 0 se q for NULL. 
 */
double getQuadraY(Quadra q);

/** 
 * @brief Retorna a largura. 
 * @return Retorna 0 se q for NULL. 
 */
double getQuadraW(Quadra q);

/** 
 * @brief Retorna a altura. 
 * @return Retorna 0 se q for NULL. 
 */
double getQuadraH(Quadra q);

/** 
 * @brief Retorna a cor de preenchimento. 
 * @return Retorna NULL se q for NULL. 
 */
const char* getQuadraCorP(Quadra q);

/** 
 * @brief Retorna a cor da borda. 
 * @return Retorna NULL se q for NULL. 
 */
const char* getQuadraCorB(Quadra q);

/** 
 * @brief Retorna a espessura da borda.
 * @return Retorna NULL se q for NULL. 
 */
const char* getQuadraSW(Quadra q);

#endif