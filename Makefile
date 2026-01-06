# ============================================================
# Giga Hash Map — static library + benchmark (C89)
# ============================================================

CC      ?= cc
AR      ?= ar
ARFLAGS := rcs
CFLAGS  := -std=c89 -O2 -Wall -Wextra -Wpedantic

# ------------------------------------------------------------
# Paths
# ------------------------------------------------------------

ARENA_DIR  := ../giga-arena
ARENA_INC  := $(ARENA_DIR)/include
ARENA_LIB  := $(ARENA_DIR)/dist/libgiga-arena.a

SRC_DIR    := src
INC_DIR    := include
BENCH_DIR  := bench

BUILD_DIR  := build
DIST_DIR   := dist

LIB        := $(DIST_DIR)/libgiga-hashmap.a
BENCH_BIN  := giga_hashmap_bench

# ------------------------------------------------------------
# Sources
# ------------------------------------------------------------

HASHMAP_SRC := $(SRC_DIR)/hashmap.c
HASHMAP_OBJ := $(BUILD_DIR)/hashmap.o
BENCH_SRC   := $(BENCH_DIR)/main.c

INCLUDES := -I$(INC_DIR) -I$(ARENA_INC)

# ------------------------------------------------------------
# Default
# ------------------------------------------------------------

.PHONY: all
all: lib bench

# ------------------------------------------------------------
# Dirs
# ------------------------------------------------------------

$(BUILD_DIR):
	@mkdir -p $@

$(DIST_DIR):
	@mkdir -p $@

# ------------------------------------------------------------
# Objects
# ------------------------------------------------------------

$(HASHMAP_OBJ): $(HASHMAP_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# ------------------------------------------------------------
# Static library
# ------------------------------------------------------------

$(LIB): $(HASHMAP_OBJ) | $(DIST_DIR)
	$(AR) $(ARFLAGS) $@ $(HASHMAP_OBJ)

.PHONY: lib
lib: $(LIB)

# ------------------------------------------------------------
# Benchmark
# ------------------------------------------------------------

.PHONY: bench
bench: $(LIB)
	$(CC) $(CFLAGS) $(INCLUDES) \
		$(BENCH_SRC) \
		$(LIB) \
		$(ARENA_LIB) \
		-o $(BENCH_BIN)

# ------------------------------------------------------------
# Clean
# ------------------------------------------------------------

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) $(DIST_DIR) $(BENCH_BIN)
