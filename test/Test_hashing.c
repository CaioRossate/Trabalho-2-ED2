#include "unity.h"
#include "hashing.h"
#include <string.h>
#include <stdlib.h>


typedef struct __attribute__((packed)) {
    char id[20];
    int  dado_extra;
    float medida;
} ItemTeste;

Hash h;

void setUp(void) {
    h = inicializarHash("teste_unidade.hf", 1, sizeof(ItemTeste));
}

void tearDown(void) {
    encerrarHash(h);
    remove("teste_unidade.hf");
}

// Testes

void test_DeveInserirEBuscarRegistroCompleto(void) {
    ItemTeste original = {"cep123", 42, 3.14f};
    ItemTeste recuperado;

    bool inseriu = inserirHash(h, &original);
    TEST_ASSERT_TRUE_MESSAGE(inseriu, "A insercao do registro falhou");

    bool encontrou = buscarHash(h, "cep123", &recuperado);
    TEST_ASSERT_TRUE_MESSAGE(encontrou, "O registro deveria ter sido encontrado");
    TEST_ASSERT_EQUAL_STRING(original.id, recuperado.id);
    TEST_ASSERT_EQUAL_INT(original.dado_extra, recuperado.dado_extra);
    TEST_ASSERT_EQUAL_FLOAT(original.medida, recuperado.medida);
}

void test_DeveAtualizarRegistroExistente(void) {
    ItemTeste item = {"id_unico", 10, 1.0f};
    ItemTeste atualizado = {"id_unico", 20, 2.0f};
    ItemTeste resultado;

    inserirHash(h, &item);
    inserirHash(h, &atualizado);
    buscarHash(h, "id_unico", &resultado);

    TEST_ASSERT_EQUAL_INT(20, resultado.dado_extra);
    TEST_ASSERT_EQUAL_FLOAT(2.0f, resultado.medida);
}

void test_NaoDeveEncontrarChaveInexistente(void) {
    ItemTeste destino;
    bool encontrou = buscarHash(h, "nao_existe", &destino);
    TEST_ASSERT_FALSE(encontrou);
}

void test_DeveRemoverRegistro(void) {
    ItemTeste item = {"para_remover", 99, 9.9f};
    ItemTeste destino;

    inserirHash(h, &item);
    TEST_ASSERT_TRUE(buscarHash(h, "para_remover", &destino));

    bool removeu = removerHash(h, "para_remover");
    TEST_ASSERT_TRUE_MESSAGE(removeu, "A remocao deveria ter sucedido");

    bool encontrou = buscarHash(h, "para_remover", &destino);
    TEST_ASSERT_FALSE_MESSAGE(encontrou, "Registro removido nao deveria ser encontrado");
}

void test_RemoverChaveInexistenteFalha(void) {
    bool removeu = removerHash(h, "fantasma");
    TEST_ASSERT_FALSE_MESSAGE(removeu, "Remover chave inexistente deveria retornar false");
}

void test_DeveForcaSplitEManterDados(void) {
    char chave[20];
    for (int i = 0; i < 25; i++) {
        ItemTeste item;
        snprintf(item.id, sizeof(item.id), "chave_%02d", i);
        item.dado_extra = i * 10;
        item.medida = (float)i;
        bool ok = inserirHash(h, &item);
        TEST_ASSERT_TRUE_MESSAGE(ok, "Insercao durante stress test falhou");
    }
    for (int i = 0; i < 25; i++) {
        snprintf(chave, sizeof(chave), "chave_%02d", i);
        ItemTeste recuperado;
        bool achou = buscarHash(h, chave, &recuperado);
        TEST_ASSERT_TRUE_MESSAGE(achou, "Registro perdido apos split");
        TEST_ASSERT_EQUAL_INT(i * 10, recuperado.dado_extra);
    }
}

typedef struct { 
    int contagem;
} CtxContar;

static void visita_contar(void* reg, void* ctx) {
    (void)reg;
    ((CtxContar*)ctx)->contagem++;
}

void test_PercorrerHashContaRegistros(void) {
    int N = 15;
    for (int i = 0; i < N; i++) {
        ItemTeste item;
        snprintf(item.id, sizeof(item.id), "reg_%02d", i);
        item.dado_extra = i;
        item.medida = 0.0f;
        inserirHash(h, &item);
    }
    CtxContar ctx = {0};
    percorrerHash(h, &ctx, visita_contar);
    TEST_ASSERT_EQUAL_INT_MESSAGE(N, ctx.contagem,
        "percorrerHash deveria visitar todos os registros exatamente uma vez");
}


int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_DeveInserirEBuscarRegistroCompleto);
    RUN_TEST(test_DeveAtualizarRegistroExistente);
    RUN_TEST(test_NaoDeveEncontrarChaveInexistente);
    RUN_TEST(test_DeveRemoverRegistro);
    RUN_TEST(test_RemoverChaveInexistenteFalha);
    RUN_TEST(test_DeveForcaSplitEManterDados);
    RUN_TEST(test_PercorrerHashContaRegistros);
    return UNITY_END();
}