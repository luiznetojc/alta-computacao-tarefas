CC = nvc
CFLAGS = -fast -mp=gpu

TARGETS = heat_cpu heat_gpu_basic heat_gpu_opt heat_gpu_enter_exit heat_gpu_loop

all: $(TARGETS)

heat_cpu: heat_cpu.c
	$(CC) $(CFLAGS) -o heat_cpu heat_cpu.c

heat_gpu_basic: heat_gpu_basic.c
	$(CC) $(CFLAGS) -o heat_gpu_basic heat_gpu_basic.c

heat_gpu_opt: heat_gpu_opt.c
	$(CC) $(CFLAGS) -o heat_gpu_opt heat_gpu_opt.c

heat_gpu_enter_exit: heat_gpu_enter_exit.c
	$(CC) $(CFLAGS) -o heat_gpu_enter_exit heat_gpu_enter_exit.c

heat_gpu_loop: heat_gpu_loop.c
	$(CC) $(CFLAGS) -o heat_gpu_loop heat_gpu_loop.c

clean:
	rm -f $(TARGETS)
