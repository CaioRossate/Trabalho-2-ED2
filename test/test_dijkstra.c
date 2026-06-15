#include "unity.h"
#include "grafo.h"
#include "dijkstra.h"

/*
 * VALORES UTILIZADOS NOS TESTES:
 *
 *   v1: 100m/5ms
 *   v2: 100m/5ms
 *   v3: 300m/2ms
 *
 * Menor distância v1->v3: v1->v2->v3 = 200m
 * Menor tempo v1->v3: v1->v3 = 150s (300/2)
 *             v1->v2->v3 = 40s (100/5 + 100/5)
 * Portanto menor tempo também é v1->v2->v3 = 40s
 */

Grafo g;

void setUp(void) {
    g = criarGrafo(5);
    inserirVertice(g, "v1", 0.0,   0.0);
    inserirVertice(g, "v2", 100.0, 0.0);
    inserirVertice(g, "v3", 200.0, 0.0);
    inserirAresta(g, "v1", "v2", "Rua_A", "-", "-", 100.0, 5.0);
    inserirAresta(g, "v2", "v3", "Rua_A", "-", "-", 100.0, 5.0);
    inserirAresta(g, "v1", "v3", "Rua_B", "-", "-", 300.0, 2.0);
}

void tearDown(void) {
    destruirGrafo(g);
}

// Distância

void test_Dijkstra_DistanciaMinimaCorreta(void) {
    Vertice v1 = buscarVertice(g, "v1");
    Vertice v3 = buscarVertice(g, "v3");

    ResultadoDijkstra r = executarDijkstra(g, v1, DIJKSTRA_DISTANCIA);
    TEST_ASSERT_NOT_NULL(r);

    // v1->v2->v3 = 200m < v1->v3 = 300m 
    TEST_ASSERT_EQUAL_FLOAT(200.0f, (float)getCusto(r, v3));
    destruirResultado(r);
}

void test_Dijkstra_DistanciaOrigem_Zero(void) {
    Vertice v1 = buscarVertice(g, "v1");
    ResultadoDijkstra r = executarDijkstra(g, v1, DIJKSTRA_DISTANCIA);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, (float)getCusto(r, v1));
    destruirResultado(r);
}

void test_Dijkstra_DistanciaVerticeIntermediario(void) {
    Vertice v1 = buscarVertice(g, "v1");
    Vertice v2 = buscarVertice(g, "v2");
    ResultadoDijkstra r = executarDijkstra(g, v1, DIJKSTRA_DISTANCIA);
    TEST_ASSERT_EQUAL_FLOAT(100.0f, (float)getCusto(r, v2));
    destruirResultado(r);
}

// Tempo

void test_Dijkstra_TempoMinimoCorreta(void) {
    Vertice v1 = buscarVertice(g, "v1");
    Vertice v3 = buscarVertice(g, "v3");

    ResultadoDijkstra r = executarDijkstra(g, v1, DIJKSTRA_TEMPO);
    TEST_ASSERT_NOT_NULL(r);

    // v1->v2->v3: 100/5 + 100/5 = 40s  <  v1->v3: 300/2 = 150s
    TEST_ASSERT_EQUAL_FLOAT(40.0f, (float)getCusto(r, v3));
    destruirResultado(r);
}

// Inacessível

void test_Dijkstra_VerticeInacessivel(void) {
    // v4 está no grafo mas sem arestas partindo de v1 até lá ou passando por ele.
    inserirVertice(g, "v4", 999.0, 999.0);
    Vertice v1 = buscarVertice(g, "v1");
    Vertice v4 = buscarVertice(g, "v4");

    ResultadoDijkstra r = executarDijkstra(g, v1, DIJKSTRA_DISTANCIA);
    TEST_ASSERT_EQUAL_FLOAT(-1.0f, (float)getCusto(r, v4));
    destruirResultado(r);
}

// Reconstrução de caminho

void test_ReconstruirCaminho_TamanhoCorreto(void) {
    Vertice v1 = buscarVertice(g, "v1");
    Vertice v3 = buscarVertice(g, "v3");

    ResultadoDijkstra r = executarDijkstra(g, v1, DIJKSTRA_DISTANCIA);
    int tam = 0;
    Vertice* caminho = reconstruirCaminho(r, v3, &tam);

    // v1 -> v2 -> v3 = 3 vértices
    TEST_ASSERT_EQUAL_INT(3, tam);
    TEST_ASSERT_NOT_NULL(caminho);
    free(caminho);
    destruirResultado(r);
}

void test_ReconstruirCaminho_VerticesCorretos(void) {
    Vertice v1 = buscarVertice(g, "v1");
    Vertice v3 = buscarVertice(g, "v3");

    ResultadoDijkstra r = executarDijkstra(g, v1, DIJKSTRA_DISTANCIA);
    int tam = 0;
    Vertice* caminho = reconstruirCaminho(r, v3, &tam);

    TEST_ASSERT_EQUAL_STRING("v1", getVerticeId(caminho[0]));
    TEST_ASSERT_EQUAL_STRING("v2", getVerticeId(caminho[1]));
    TEST_ASSERT_EQUAL_STRING("v3", getVerticeId(caminho[2]));
    free(caminho);
    destruirResultado(r);
}

void test_ReconstruirCaminho_Inacessivel_RetornaNull(void) {
    inserirVertice(g, "v4", 999.0, 999.0);
    Vertice v1 = buscarVertice(g, "v1");
    Vertice v4 = buscarVertice(g, "v4");

    ResultadoDijkstra r = executarDijkstra(g, v1, DIJKSTRA_DISTANCIA);
    int tam = 0;
    Vertice* caminho = reconstruirCaminho(r, v4, &tam);

    TEST_ASSERT_NULL(caminho);
    TEST_ASSERT_EQUAL_INT(0, tam);
    destruirResultado(r);
}


int main(void) {
    UNITY_BEGIN();

    // Distância
    RUN_TEST(test_Dijkstra_DistanciaMinimaCorreta);
    RUN_TEST(test_Dijkstra_DistanciaOrigem_Zero);
    RUN_TEST(test_Dijkstra_DistanciaVerticeIntermediario);

    // Tempo
    RUN_TEST(test_Dijkstra_TempoMinimoCorreta);

    // Inacessível
    RUN_TEST(test_Dijkstra_VerticeInacessivel);

    // Reconstrução
    RUN_TEST(test_ReconstruirCaminho_TamanhoCorreto);
    RUN_TEST(test_ReconstruirCaminho_VerticesCorretos);
    RUN_TEST(test_ReconstruirCaminho_Inacessivel_RetornaNull);

    return UNITY_END();
}