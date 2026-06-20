CC=nvc
CFLAGS=-O3
OMP_FLAG_CPU=-mp
OMP_FLAG_GPU=-mp=gpu

all: heat_cpu heat_gpu_basic heat_gpu_opt

heat_cpu: heat_cpu.c
	$(CC) $(CFLAGS) $(OMP_FLAG_CPU) -o $@ $^

heat_gpu_basic: heat_gpu_basic.c
	$(CC) $(CFLAGS) $(OMP_FLAG_GPU) -o $@ $^

heat_gpu_opt: heat_gpu_opt.c
	$(CC) $(CFLAGS) $(OMP_FLAG_GPU) -o $@ $^

clean:
	rm -f heat_cpu heat_gpu_basic heat_gpu_opt
