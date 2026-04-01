CC ?= cc
CFLAGS ?= -O2 -Wall -Wextra -std=c11

UNAME_S := $(shell uname -s)
OPENMP_CFLAGS :=
OPENMP_LDFLAGS :=

ifeq ($(UNAME_S),Darwin)
BREW_PREFIX := $(shell brew --prefix libomp 2>/dev/null)
ifneq ($(BREW_PREFIX),)
OPENMP_CFLAGS += -Xpreprocessor -fopenmp -I$(BREW_PREFIX)/include
OPENMP_LDFLAGS += -L$(BREW_PREFIX)/lib -lomp
else
OPENMP_CFLAGS += -Xpreprocessor -fopenmp
OPENMP_LDFLAGS += -lomp
endif
else
OPENMP_CFLAGS += -fopenmp
OPENMP_LDFLAGS += -fopenmp
endif

SEQ_SRC := primos_sequencial.c
OMP_SRC := primos_parallel.c
DYNAMIC_SRC := primos_parallel-dynamic.c

SEQ_BIN := $(SEQ_SRC:.c=)
OMP_BIN := $(OMP_SRC:.c=)
BIN := $(SEQ_BIN) $(OMP_BIN)

all: $(BIN)

$(SEQ_BIN): %: %.c
	$(CC) $(CFLAGS) $< -o $@

$(OMP_BIN): %: %.c
	$(CC) $(CFLAGS) $(OPENMP_CFLAGS) $< -o $@ $(OPENMP_LDFLAGS)

run-seq: primos_sequencial
	./primos_sequencial 1000000

run-par: primos_parallel
	./primos_parallel 1000000

dynamic: primos_parallel-dynamic
	./primos_parallel-dynamic 1000000

$(DYNAMIC_SRC:.c=): %: %.c
	$(CC) $(CFLAGS) $(OPENMP_CFLAGS) $< -o $@ $(OPENMP_LDFLAGS)

clean:
	rm -f $(BIN)

.PHONY: all clean run-seq run-par dynamic
