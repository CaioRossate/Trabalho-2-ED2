#ifndef HASHING_H
#define HASHING_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @file hashing.h
 * @brief Gerenciador de Hashfile Dinâmico (Extensível) em disco para Bitnópolis.
 * @details Implementa a persistência de dados em arquivos binários (.hf).
 * Por ser generico pode armazenar Quadras e Habitantes.
 */


typedef void* Hash;

/**
 * @brief Inicializa ou carrega um Hashfile em disco.
 * @param nome_arquivo Caminho/nome do arquivo físico no disco (deve terminar em .hf).
 * @param profundidade_inicial A profundidade global para o início do diretório (ex: 1).
 * @param tamanho_registro O tamanho exato em bytes da struct que será armazenada (ex: sizeof(struct quadra)).
 * @return Retorna um ponteiro (Hash) para o manipulador da estrutura, ou NULL em caso de falha na criação/leitura.
 */
Hash inicializarHash(const char* nome_arquivo, int profundidade_inicial, size_t tamanho_registro);

/**
 * @brief Insere um registro no Hashfile ou atualiza se a chave já existir.
 * @param h O manipulador do Hashfile retornado por inicializarHash.
 * @param dado Ponteiro para a struct contendo os dados. O primeiro campo DEVE ser a chave (char[20]).
 * @return true se o dado foi persistido com sucesso no disco; false em caso de erro.
 */
bool inserirHash(Hash h, void* dado);

/**
 * @brief Busca um registro no Hashfile através de sua chave.
 * @param h O manipulador do Hashfile.
 * @param chave A string de busca (CEP da quadra ou CPF do habitante).
 * @param destino Ponteiro para uma struct local onde os dados encontrados serão copiados.
 * @return true se a chave foi encontrada; false caso contrário.
 */
bool buscarHash(Hash h, char* chave, void* destino);

/**
 * @brief Gera um relatório legível do estado atual do Hashfile.
 * * Cria um arquivo de texto (.hfd) contendo a representação esquemática do conteúdo,
 * incluindo buckets, ocupação e expansões realizadas.
 * * @param h O manipulador do Hashfile.
 * @param nome_arquivo_relatorio Nome do arquivo .hfd a ser gerado.
 */
void gerarRelatorioHash(Hash h, const char* nome_arquivo_relatorio);

/**
 * @brief Percorre todos os registros do hash e aplica uma função neles.
 * @param h O manipulador do Hash.
 * @param contexto Ponteiro para qualquer dado extra (ex: um contador).
 * @param funcao_visita Função que será chamada para cada registro encontrado.
 */
void percorrerHash(Hash h, void* contexto, void (*funcao_visita)(void* registro, void* contexto));

/**
 * @brief Retorna a profundidade global atual do diretório.
 * @param h O manipulador do Hash.
 * @return A profundidade global do diretório.
 */
int getProfundidadeGlobal(Hash h);

/**
 * @brief Retorna o número total de buckets alocados no arquivo.
 * @param h O manipulador do Hash.
 * @return O número total de buckets alocados.
 */
int getQuantidadeBuckets(Hash h);

/**
 * @brief Remove um registro do Hashfile com base na chave fornecida.
 * @param h O manipulador do Hashfile.
 * @param chave A string de busca (CEP da quadra ou CPF do habitante) a ser removida.
 * @return true se a chave foi encontrada e removida; false caso contrário.
 */
bool removerHash(Hash h, char* chave);

/**
 * @brief Fecha o arquivo binário e libera a memória do diretório.
 * @param h O manipulador do Hashfile.
 */
void encerrarHash(Hash h);

#endif