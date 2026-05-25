#ifndef QRY_H
#define QRY_H

#include <stdio.h>
#include "hashing.h"

/**
 * @file qry.h
 * @brief Execução dos comandos de consulta definidos no arquivo .qry
 *
 * Comandos:
 *   o?     — armazena referência geográfica num registrador
 *   mvm    — atualiza velocidade média das arestas numa região
 *   regs   — calcula componentes conexos com vm < vl
 *   exp    — árvore geradora mínima; aumenta vm das arestas lentas
 *   p?     — menor caminho entre dois registradores (distância e tempo)
 */

/**
 * @brief Faz uma pré-leitura do .qry e coleta todos os CEPs removidos por "rq".
 * @param path_qry Caminho para o arquivo .qry.
 * @param ceps_out Array de strings alocado pelo chamador.
 * @param max_ceps Capacidade máxima do array.
 * @return Número de CEPs coletados.
 */
int coletarCepsRemovidos(const char* path_qry, char ceps_out[][20], int max_ceps);

/**
 * @brief Processa o arquivo .qry e executa todos os comandos de consulta.
 * @param path_qry Caminho para o arquivo .qry.
 * @param h_q Hash de quadras.
 * @param grafo Ponteiro opaco para o grafo viário (void* para evitar dependência circular).
 * @param fTxt Arquivo de saída textual.
 * @param fSvg Arquivo SVG de saída.
 */
void processarArquivoQry(const char* path_qry, Hash h_q, void* grafo, FILE* fTxt, FILE* fSvg);

/**
 * @brief Armazena a posição geográfica de um endereço num registrador (Comando: o?).
 * @details O registrador é um índice 0..10. A posição é calculada a partir do CEP, face e número, consultando as arestas adjacentes à quadra no grafo.
 * @param grafo Ponteiro para o grafo viário.
 * @param reg Índice do registrador (0..10).
 * @param cep CEP da quadra de referência.
 * @param face Face da quadra ('N','S','L','O').
 * @param num Número no logradouro.
 * @param fTxt Arquivo de saída textual.
 * @param fSvg Arquivo SVG de saída.
 */
void comando_o(void* grafo, int reg, char* cep, char face, double num, FILE* fTxt, FILE* fSvg);

/**
 * @brief Atualiza a velocidade média das arestas dentro de uma região (Comando: mvm).
 * @param grafo Ponteiro para o grafo viário.
 * @param v Nova velocidade média (m/s).
 * @param x Coordenada X da região.
 * @param y Coordenada Y da região.
 * @param w Largura da região.
 * @param h Altura da região.
 */
void comando_mvm(void* grafo, double v, double x, double y, double w, double h);

/**
 * @brief Calcula componentes conexos para arestas com vm < vl (Comando: regs).
 * @param grafo Ponteiro para o grafo viário.
 * @param vl Velocidade-limite para considerar trecho insuficiente.
 * @param fTxt Arquivo de saída textual (número de componentes).
 * @param fSvg Arquivo SVG (bounding boxes coloridos com 50% de transparência).
 */
void comando_regs(void* grafo, double vl, FILE* fTxt, FILE* fSvg);

/**
 * @brief Calcula AGM e aumenta vm das arestas lentas em 50% (Comando: exp).
 * @param grafo Ponteiro para o grafo viário.
 * @param vl Limiar de velocidade para selecionar arestas a expandir.
 * @param fSvg Arquivo SVG (arestas selecionadas em vermelho e linha grossa).
 */
void comando_exp(void* grafo, double vl, FILE* fSvg);

/**
 * @brief Determina o melhor trajeto entre dois registradores (Comando: p?).
 * @details Calcula dois percursos: mais curto (menor distância) e mais rápido
 * (menor tempo). Ambos são desenhados no SVG com animação e marcados com placas I e F nos extremos.
 * @param grafo Ponteiro para o grafo viário.
 * @param reg1 Registrador de origem (0..10).
 * @param reg2 Registrador de destino (0..10).
 * @param cc Cor do percurso mais curto.
 * @param cr Cor do percurso mais rápido.
 * @param fTxt Arquivo de saída textual (descrição do percurso).
 * @param fSvg Arquivo SVG (percursos animados).
 */
void comando_p(void* grafo, int reg1, int reg2, char* cc, char* cr, FILE* fTxt, FILE* fSvg);

#endif