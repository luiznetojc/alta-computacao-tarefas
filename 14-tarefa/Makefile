CC = mpicc
CFLAGS = -O2 -Wall -Wextra

TARGETS = pingpong_send pingpong_ssend pingpong_bsend pingpong_rsend

all: $(TARGETS)

pingpong_send: pingpong_send.c
	$(CC) $(CFLAGS) -o $@ $<

pingpong_ssend: pingpong_ssend.c
	$(CC) $(CFLAGS) -o $@ $<

pingpong_bsend: pingpong_bsend.c
	$(CC) $(CFLAGS) -o $@ $<

pingpong_rsend: pingpong_rsend.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)