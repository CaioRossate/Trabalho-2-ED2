CC     = gcc
CFLAGS = -std=c99 -DUNITY_INCLUDE_DOUBLE -Wall -Wextra -I../include -I../unity -fstack-protector-all
LDFLAGS = -lm

SRC_DIR   := .
BUILD_DIR := ../build
BIN       := ted

# Arquivos fonte do projeto principal
SRC_FILES := $(shell find $(SRC_DIR) -name '*.c' ! -name 'main.c')
OBJ_FILES := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRC_FILES))

# Arquivos de teste
TEST_DIR  := ../test
TEST_SRCS := $(wildcard $(TEST_DIR)/*.c)
TEST_BINS := $(patsubst $(TEST_DIR)/%.c, ./%, $(TEST_SRCS))

# Unity
UNITY_SRCS := ../unity/unity.c
UNITY_OBJS := $(BUILD_DIR)/unity/unity.o

.PHONY: all clean tests run_tests

all: $(BIN)

# Linka o executável principal
$(BIN): $(OBJ_FILES) $(BUILD_DIR)/main.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compila main.c separadamente
$(BUILD_DIR)/main.o: $(SRC_DIR)/main.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Regra genérica para os demais .c do projeto
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Compilação do Unity
$(UNITY_OBJS): $(UNITY_SRCS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Cada arquivo de teste vira um binário separado
./%: $(TEST_DIR)/%.c $(OBJ_FILES) $(UNITY_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compila todos os testes
tests: $(TEST_BINS)

# Compila e executa todos os testes
run_tests: tests
	@echo "=== Executando testes ==="
	@for test in $(TEST_BINS); do \
		echo "--- $$test ---"; \
		./$$test; \
	done

clean:
	rm -rf $(BUILD_DIR) $(BIN) $(TEST_BINS)
	rm -f *.hf *.hfc *.hfd