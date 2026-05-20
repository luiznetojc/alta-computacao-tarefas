CC = mpicc
CFLAGS = -O2 -Wall -Wextra

TARGETS = heat_blocking heat_nonblocking heat_overlap

all: $(TARGETS)

heat_blocking: heat_blocking.c
	$(CC) $(CFLAGS) -o $@ $<

heat_nonblocking: heat_nonblocking.c
	$(CC) $(CFLAGS) -o $@ $<

heat_overlap: heat_overlap.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)