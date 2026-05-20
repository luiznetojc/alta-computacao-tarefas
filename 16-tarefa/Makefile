CC = mpicc
CFLAGS = -O2 -Wall -Wextra

TARGETS = primos_master_worker

all: $(TARGETS)

primos_master_worker: primos_master_worker.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)