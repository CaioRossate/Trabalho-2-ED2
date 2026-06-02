#include "unity.h"
#include "grafo.h"
#include <string.h>

Grafo g;

void setUp(void) {
    g = criarGrafo(10);
}

void tearDown(void) {
    destruirGrafo(g);
}


// Criando o grafo


void test_CriarGrafo_NaoNulo(void) {
    TEST_ASSERT_NOT_NULL(g);
}

void test_GrafoVazio_ZeroVertices(void) {
    TEST_ASSERT_EQUAL_INT(0, getNumVertices(g));
}

void test_GrafoVazio_ZeroArestas(void) {
    TEST_ASSERT_EQUAL_INT(0, getNumArestas(g));
}

// Testes dos vertices

void test_InserirVertice_RetornaNaoNulo(void) {
    Vertice v = inserirVertice(g, "v1", 10.0, 20.0);
    TEST_ASSERT_NOT_NULL(v);
}

void test_InserirVertice_DadosCorretos(void) {
    Vertice v = inserirVertice(g, "v1", 10.0, 20.0);
    TEST_ASSERT_EQUAL_STRING("v1", getVerticeId(v));
    TEST_ASSERT_EQUAL_FLOAT(10.0f, (float)getVerticeX(v));
    TEST_ASSERT_EQUAL_FLOAT(20.0f, (float)getVerticeY(v));
}

void test_InserirVertice_IncrementaContagem(void) {
    inserirVertice(g, "v1", 0.0, 0.0);
    inserirVertice(g, "v2", 1.0, 1.0);
    inserirVertice(g, "v3", 2.0, 2.0);
    TEST_ASSERT_EQUAL_INT(3, getNumVertices(g));
}

void test_BuscarVertice_EncontradoAposInsercao(void) {
    inserirVertice(g, "v42", 5.0, 7.0);
    Vertice v = buscarVertice(g, "v42");
    TEST_ASSERT_NOT_NULL(v);
    TEST_ASSERT_EQUAL_STRING("v42", getVerticeId(v));
}

void test_BuscarVertice_NaoEncontrado(void) {
    Vertice v = buscarVertice(g, "inexistente");
    TEST_ASSERT_NULL(v);
}

void test_BuscarVertice_MultiplosDados(void) {
    inserirVertice(g, "va", 1.0, 2.0);
    inserirVertice(g, "vb", 3.0, 4.0);
    inserirVertice(g, "vc", 5.0, 6.0);

    Vertice va = buscarVertice(g, "va");
    Vertice vb = buscarVertice(g, "vb");
    Vertice vc = buscarVertice(g, "vc");

    TEST_ASSERT_NOT_NULL(va);
    TEST_ASSERT_NOT_NULL(vb);
    TEST_ASSERT_NOT_NULL(vc);
    TEST_ASSERT_EQUAL_FLOAT(1.0f, (float)getVerticeX(va));
    TEST_ASSERT_EQUAL_FLOAT(4.0f, (float)getVerticeY(vb));
    TEST_ASSERT_EQUAL_FLOAT(5.0f, (float)getVerticeX(vc));
}

// Testes das arestas

void test_InserirAresta_RetornaNaoNulo(void) {
    inserirVertice(g, "v1", 0.0, 0.0);
    inserirVertice(g, "v2", 100.0, 0.0);
    Aresta a = inserirAresta(g, "v1", "v2", "Rua_A", "cep01", "-", 100.0, 5.0);
    TEST_ASSERT_NOT_NULL(a);
}

void test_InserirAresta_DadosCorretos(void) {
    inserirVertice(g, "v1", 0.0, 0.0);
    inserirVertice(g, "v2", 100.0, 0.0);
    Aresta a = inserirAresta(g, "v1", "v2", "Rua_A", "cep01", "cep02", 100.0, 5.0);

    TEST_ASSERT_EQUAL_STRING("Rua_A",  getArestaNome(a));
    TEST_ASSERT_EQUAL_STRING("cep01",  getArestaLdir(a));
    TEST_ASSERT_EQUAL_STRING("cep02",  getArestaLesq(a));
    TEST_ASSERT_EQUAL_FLOAT(100.0f, (float)getArestaCmp(a));
    TEST_ASSERT_EQUAL_FLOAT(5.0f,   (float)getArestaVm(a));
}

void test_InserirAresta_IncrementaContagem(void) {
    inserirVertice(g, "v1", 0.0, 0.0);
    inserirVertice(g, "v2", 1.0, 0.0);
    inserirVertice(g, "v3", 2.0, 0.0);
    inserirAresta(g, "v1", "v2", "Rua_A", "-", "-", 100.0, 5.0);
    inserirAresta(g, "v2", "v3", "Rua_A", "-", "-", 100.0, 5.0);
    TEST_ASSERT_EQUAL_INT(2, getNumArestas(g));
}

void test_InserirAresta_VerticesOrigemDestino(void) {
    inserirVertice(g, "v1", 0.0,   0.0);
    inserirVertice(g, "v2", 100.0, 0.0);
    Aresta a = inserirAresta(g, "v1", "v2", "Rua_X", "-", "-", 50.0, 3.0);

    Vertice origem  = getArestaOrigem(a);
    Vertice destino = getArestaDestino(a);
    TEST_ASSERT_EQUAL_STRING("v1", getVerticeId(origem));
    TEST_ASSERT_EQUAL_STRING("v2", getVerticeId(destino));
}

void test_InserirAresta_VerticeInexistenteRetornaNull(void) {
    inserirVertice(g, "v1", 0.0, 0.0);
    /* v2 não existe */
    Aresta a = inserirAresta(g, "v1", "fantasma", "Rua_X", "-", "-", 50.0, 3.0);
    TEST_ASSERT_NULL(a);
}

void test_SetArestaVm_AtualizaVelocidade(void) {
    inserirVertice(g, "v1", 0.0,   0.0);
    inserirVertice(g, "v2", 100.0, 0.0);
    Aresta a = inserirAresta(g, "v1", "v2", "Rua_A", "-", "-", 100.0, 5.0);
    setArestaVm(a, 10.0);
    TEST_ASSERT_EQUAL_FLOAT(10.0f, (float)getArestaVm(a));
}

// Testes dos iteradores

typedef struct { int contagem; } CtxContar;

static void contarAresta(Aresta a, void* ctx) {
    (void)a;
    ((CtxContar*)ctx)->contagem++;
}

static void contarVertice(Vertice v, void* ctx) {
    (void)v;
    ((CtxContar*)ctx)->contagem++;
}

void test_IterarAdjacentes_ContaCorreto(void) {
    inserirVertice(g, "v1", 0.0, 0.0);
    inserirVertice(g, "v2", 1.0, 0.0);
    inserirVertice(g, "v3", 2.0, 0.0);
    inserirAresta(g, "v1", "v2", "Rua_A", "-", "-", 100.0, 5.0);
    inserirAresta(g, "v1", "v3", "Rua_B", "-", "-", 200.0, 5.0);

    Vertice v1 = buscarVertice(g, "v1");
    CtxContar ctx = {0};
    iterarAdjacentes(g, v1, &ctx, contarAresta);
    TEST_ASSERT_EQUAL_INT(2, ctx.contagem);
}

void test_IterarVertices_ContaTodos(void) {
    inserirVertice(g, "v1", 0.0, 0.0);
    inserirVertice(g, "v2", 1.0, 0.0);
    inserirVertice(g, "v3", 2.0, 0.0);
    CtxContar ctx = {0};
    iterarVertices(g, &ctx, contarVertice);
    TEST_ASSERT_EQUAL_INT(3, ctx.contagem);
}

void test_IterarArestas_ContaTodas(void) {
    inserirVertice(g, "v1", 0.0, 0.0);
    inserirVertice(g, "v2", 1.0, 0.0);
    inserirVertice(g, "v3", 2.0, 0.0);
    inserirAresta(g, "v1", "v2", "Rua_A", "-", "-", 100.0, 4.0);
    inserirAresta(g, "v2", "v3", "Rua_A", "-", "-", 100.0, 4.0);
    inserirAresta(g, "v3", "v1", "Rua_B", "-", "-", 200.0, 3.0);
    CtxContar ctx = {0};
    iterarArestas(g, &ctx, contarAresta);
    TEST_ASSERT_EQUAL_INT(3, ctx.contagem);
}


int main(void) {
    UNITY_BEGIN();

    // Criação 
    RUN_TEST(test_CriarGrafo_NaoNulo);
    RUN_TEST(test_GrafoVazio_ZeroVertices);
    RUN_TEST(test_GrafoVazio_ZeroArestas);

    // Vértices
    RUN_TEST(test_InserirVertice_RetornaNaoNulo);
    RUN_TEST(test_InserirVertice_DadosCorretos);
    RUN_TEST(test_InserirVertice_IncrementaContagem);
    RUN_TEST(test_BuscarVertice_EncontradoAposInsercao);
    RUN_TEST(test_BuscarVertice_NaoEncontrado);
    RUN_TEST(test_BuscarVertice_MultiplosDados);

    // Arestas
    RUN_TEST(test_InserirAresta_RetornaNaoNulo);
    RUN_TEST(test_InserirAresta_DadosCorretos);
    RUN_TEST(test_InserirAresta_IncrementaContagem);
    RUN_TEST(test_InserirAresta_VerticesOrigemDestino);
    RUN_TEST(test_InserirAresta_VerticeInexistenteRetornaNull);
    RUN_TEST(test_SetArestaVm_AtualizaVelocidade);

    // Iteradores
    RUN_TEST(test_IterarAdjacentes_ContaCorreto);
    RUN_TEST(test_IterarVertices_ContaTodos);
    RUN_TEST(test_IterarArestas_ContaTodas);

    return UNITY_END();
}