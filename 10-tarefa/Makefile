CC = gcc
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

TARGETS := pi_randr_critical pi_randr_atomic pi_randr_private_critical pi_randr_private_atomic pi_randr_reduction

all: $(TARGETS)

pi_randr_critical: pi_randr_critical.c
	$(CC) $(CFLAGS) $(OPENMP_CFLAGS) $< -o $@ $(OPENMP_LDFLAGS)

pi_randr_atomic: pi_randr_atomic.c
	$(CC) $(CFLAGS) $(OPENMP_CFLAGS) $< -o $@ $(OPENMP_LDFLAGS)

pi_randr_private_critical: pi_randr_private_critical.c
	$(CC) $(CFLAGS) $(OPENMP_CFLAGS) $< -o $@ $(OPENMP_LDFLAGS)

pi_randr_private_atomic: pi_randr_private_atomic.c
	$(CC) $(CFLAGS) $(OPENMP_CFLAGS) $< -o $@ $(OPENMP_LDFLAGS)

pi_randr_reduction: pi_randr_reduction.c
	$(CC) $(CFLAGS) $(OPENMP_CFLAGS) $< -o $@ $(OPENMP_LDFLAGS)

clean:
	rm -f $(TARGETS)

.PHONY: all clean