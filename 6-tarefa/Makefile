CC = gcc
CFLAGS = -O2 -Wall -Wextra -std=c11 -fopenmp

TARGETS = pi-sequencial pi-parallel-wrong pi-parallel-critic pi-parallel-reduction

all: $(TARGETS)

pi-sequencial: pi-sequencial.c
	$(CC) $(CFLAGS) $< -o $@

pi-parallel-wrong: pi-parallel-wrong.c
	$(CC) $(CFLAGS) $< -o $@

pi-parallel-critic: pi-parallel-critic.c
	$(CC) $(CFLAGS) $< -o $@

pi-parallel-reduction: pi-parallel-reduction.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(TARGETS)

.PHONY: all clean
