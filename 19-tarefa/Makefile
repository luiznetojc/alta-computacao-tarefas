CC = nvc
CFLAGS = -fast -mp=gpu

TARGETS = vadd_cpu vadd_slide27 vadd_slide48

all: $(TARGETS)

vadd_cpu: vadd_cpu.c
	$(CC) $(CFLAGS) -o vadd_cpu vadd_cpu.c

vadd_slide27: vadd_slide27.c
	$(CC) $(CFLAGS) -o vadd_slide27 vadd_slide27.c

vadd_slide48: vadd_slide48.c
	$(CC) $(CFLAGS) -o vadd_slide48 vadd_slide48.c

clean:
	rm -f $(TARGETS)
