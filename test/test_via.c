#include "unity.h"
#include "via.h"
#include "grafo.h"
#include <stdio.h>
#include <stdlib.h>

#define ARQUIVO_TESTE "teste_via.via"

// Cria um arquivo .via temporário para os testes
static void criarArquivoVia(const char* conteudo) {
    FILE* f = fopen(ARQUIVO_TESTE, "w");
    fprintf(f, "%s", conteudo);
    fclose(f);
}

void setUp(void) {

}
void tearDown(void) {
    remove(ARQUIVO_TESTE); 
}

// pega a primeira aresta encontrada
static void capturar(Aresta a, void* ctx) {
    Aresta* alvo = (Aresta*)ctx;
    if (!(*alvo)) *alvo = a;
}

// Leitura

void test_Via_ArquivoInexistente_RetornaNull(void) {
    Grafo g = processarArquivoVia("nao_existe.via");
    TEST_ASSERT_NULL(g);
}

void test_Via_LerVertices(void) {
    criarArquivoVia("3\n" "v v1 10.0 10.0\n" "v v2 110.0 10.0\n" "v v3 210.0 10.0\n");
    Grafo g = processarArquivoVia(ARQUIVO_TESTE);
    TEST_ASSERT_NOT_NULL(g);
    TEST_ASSERT_EQUAL_INT(3, getNumVertices(g));
    destruirGrafo(g);
}

void test_Via_LerArestas(void) {
    criarArquivoVia("3\n" "v v1 10.0 10.0\n" "v v2 110.0 10.0\n" "v v3 210.0 10.0\n" 
    "e v1 v2 - cep1 100.0 5.0 Rua_A\n" "e v2 v3 cep1 - 100.0 4.0 Rua_A\n");
    Grafo g = processarArquivoVia(ARQUIVO_TESTE);
    TEST_ASSERT_NOT_NULL(g);
    TEST_ASSERT_EQUAL_INT(2, getNumArestas(g));
    destruirGrafo(g);
}

void test_Via_DadosVerticeCorretos(void) {
    criarArquivoVia("2\n" "v abc 55.5 77.7\n" "v xyz 11.1 22.2\n");
    Grafo g = processarArquivoVia(ARQUIVO_TESTE);
    Vertice v = buscarVertice(g, "abc");
    TEST_ASSERT_NOT_NULL(v);
    TEST_ASSERT_EQUAL_STRING("abc", getVerticeId(v));
    TEST_ASSERT_EQUAL_FLOAT(55.5f, (float)getVerticeX(v));
    TEST_ASSERT_EQUAL_FLOAT(77.7f, (float)getVerticeY(v));
    destruirGrafo(g);
}

void test_Via_DadosArestaCorretos(void) {
    criarArquivoVia("2\n" "v v1 0.0 0.0\n" "v v2 100.0 0.0\n" 
    "e v1 v2 cep01 cep02 150.0 6.5 Av_Principal\n");
    Grafo g = processarArquivoVia(ARQUIVO_TESTE);
    TEST_ASSERT_EQUAL_INT(1, getNumArestas(g));

    // Verifica os dados via iteração
    Vertice v1 = buscarVertice(g, "v1");
    TEST_ASSERT_NOT_NULL(v1);

    // Coleta a aresta pelo iterador de adjacentes 
    Aresta aresta_encontrada = NULL;
    
    
    iterarAdjacentes(g, v1, &aresta_encontrada, capturar);

    TEST_ASSERT_NOT_NULL(aresta_encontrada);
    TEST_ASSERT_EQUAL_STRING("Av_Principal", getArestaNome(aresta_encontrada));
    TEST_ASSERT_EQUAL_STRING("cep01", getArestaLdir(aresta_encontrada));
    TEST_ASSERT_EQUAL_STRING("cep02", getArestaLesq(aresta_encontrada));
    TEST_ASSERT_EQUAL_FLOAT(150.0f, (float)getArestaCmp(aresta_encontrada));
    TEST_ASSERT_EQUAL_FLOAT(6.5f, (float)getArestaVm(aresta_encontrada));

    destruirGrafo(g);
}

void test_Via_ArestaComHifen_LadoAusente(void) {
    criarArquivoVia("2\n" "v v1 0.0 0.0\n" "v v2 70.0 0.0\n" 
    "e v1 v2 - cep1 70.0 3.5 Rua_Belo_Horizonte\n");
    Grafo g = processarArquivoVia(ARQUIVO_TESTE);

    Vertice v1 = buscarVertice(g, "v1");
    Aresta a = NULL;
    
    iterarAdjacentes(g, v1, &a, capturar);

    TEST_ASSERT_NOT_NULL(a);
    TEST_ASSERT_EQUAL_STRING("-", getArestaLdir(a));
    TEST_ASSERT_EQUAL_STRING("cep1", getArestaLesq(a));
    destruirGrafo(g);
}

void test_Via_ArestaVerticeInexistente_Ignorada(void) {
    criarArquivoVia("1\n" "v v1 0.0 0.0\n" "e v1 fantasma - - 100.0 5.0 Rua_X\n");
    Grafo g = processarArquivoVia(ARQUIVO_TESTE);
    // Aresta com vértice inexistente deve ser ignorada
    TEST_ASSERT_EQUAL_INT(0, getNumArestas(g));
    destruirGrafo(g);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_Via_ArquivoInexistente_RetornaNull);
    RUN_TEST(test_Via_LerVertices);
    RUN_TEST(test_Via_LerArestas);
    RUN_TEST(test_Via_DadosVerticeCorretos);
    RUN_TEST(test_Via_DadosArestaCorretos);
    RUN_TEST(test_Via_ArestaComHifen_LadoAusente);
    RUN_TEST(test_Via_ArestaVerticeInexistente_Ignorada);

    return UNITY_END();
}