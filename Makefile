# ============================================================
# Makefile — Giga Hash Map (Arena-backed Benchmark)
# ============================================================

CC      ?= cc
CFLAGS  := -std=c89 -O2 -Wall -Wextra -Wpedantic

# ------------------------------------------------------------
# Paths
# ------------------------------------------------------------

DEPS_DIR    := deps
ARENA_DIR   := $(DEPS_DIR)/giga-arena
ARENA_REPO  := https://github.com/DonnieTD/giga-arena
ARENA_STAMP := $(ARENA_DIR)/.fetched
ARENA_SRC   := $(ARENA_DIR)/main.c

HASHMAP_SRC := main.c
BIN := giga_hashmap_bench

# ------------------------------------------------------------
# Default target
# ------------------------------------------------------------

.PHONY: all
all: $(BIN)

# ------------------------------------------------------------
# Fetch giga-arena (stamp-based)
# ------------------------------------------------------------

$(ARENA_STAMP):
	@echo "Fetching giga-arena..."
	@mkdir -p $(DEPS_DIR)
	@if [ ! -d "$(ARENA_DIR)" ]; then \
		git clone $(ARENA_REPO) $(ARENA_DIR); \
	fi
	@touch $(ARENA_STAMP)

# ------------------------------------------------------------
# Teach make that main.c comes from the clone
# ------------------------------------------------------------

$(ARENA_SRC): $(ARENA_STAMP)
	@true

# ------------------------------------------------------------
# Build objects
# ------------------------------------------------------------

arena.o: $(ARENA_SRC)
	$(CC) $(CFLAGS) -DGIGA_ARENA_NO_MAIN -c $(ARENA_SRC) -o arena.o

hashmap.o: $(HASHMAP_SRC)
	$(CC) $(CFLAGS) -c $(HASHMAP_SRC) -o hashmap.o

# ------------------------------------------------------------
# Link
# ------------------------------------------------------------

$(BIN): arena.o hashmap.o
	$(CC) arena.o hashmap.o -o $(BIN)

# ------------------------------------------------------------
# Run benchmark
# ------------------------------------------------------------

.PHONY: run
run: $(BIN)
	./$(BIN)

# ------------------------------------------------------------
# Clean
# ------------------------------------------------------------

.PHONY: clean
clean:
	rm -f *.o $(BIN)

# ------------------------------------------------------------
# Remove dependencies
# ------------------------------------------------------------

.PHONY: distclean
distclean: clean
	rm -rf $(DEPS_DIR)

