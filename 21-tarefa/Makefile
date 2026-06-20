CC = nvc
CFLAGS = -fast -mp=gpu

TARGETS = heat_cpu heat_gpu_basic heat_gpu_opt

all: $(TARGETS)

heat_cpu: heat_cpu.c
	$(CC) $(CFLAGS) -o heat_cpu heat_cpu.c

heat_gpu_basic: heat_gpu_basic.c
	$(CC) $(CFLAGS) -o heat_gpu_basic heat_gpu_basic.c

heat_gpu_opt: heat_gpu_opt.c
	$(CC) $(CFLAGS) -o heat_gpu_opt heat_gpu_opt.c

clean:
	rm -f $(TARGETS)
