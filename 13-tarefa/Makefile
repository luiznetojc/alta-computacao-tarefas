CC = gcc
CFLAGS ?= -O2 -Wall -Wextra -std=c11
LDFLAGS = -lm

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

TARGETS := navier_serial navier_static_collapse navier_static navier_dynamic navier_guided

all: $(TARGETS)

navier_serial: navier_serial.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

navier_static: navier_static.c
	$(CC) $(CFLAGS) $(OPENMP_CFLAGS) $< -o $@ $(OPENMP_LDFLAGS) $(LDFLAGS)

navier_dynamic: navier_dynamic.c
	$(CC) $(CFLAGS) $(OPENMP_CFLAGS) $< -o $@ $(OPENMP_LDFLAGS) $(LDFLAGS)

navier_guided: navier_guided.c
	$(CC) $(CFLAGS) $(OPENMP_CFLAGS) $< -o $@ $(OPENMP_LDFLAGS) $(LDFLAGS)

navier_static_collapse: navier_static_collapse.c
	$(CC) $(CFLAGS) $(OPENMP_CFLAGS) $< -o $@ $(OPENMP_LDFLAGS) $(LDFLAGS)

clean:
	rm -f $(TARGETS)

.PHONY: all clean
