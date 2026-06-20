# Relatório: Prática Introdutória 2 - Tarefa Extra (Derivada Numérica em CUDA)

## 1. Introdução
O objetivo desta tarefa foi implementar um programa em CUDA para calcular a derivada numérica de uma função matemática utilizando o método das diferenças finitas. A função escolhida para o experimento foi $f(x) = \sin(x)$. O método das diferenças finitas centrais foi utilizado, aproximando a derivada como:

$$f'(x) \approx \frac{f(x + h) - f(x - h)}{2h}$$

onde $h = 10^{-5}$ foi o passo utilizado.

## 2. Metodologia
A implementação consistiu em duas partes para possibilitar a comparação de desempenho e corretude:
1. **Versão Sequencial (CPU):** Um laço de repetição itera sobre todos os pontos do domínio calculando a derivada.
2. **Versão Paralela (GPU - CUDA):** A paralelização foi feita alocando uma *thread* para cada ponto do domínio. O *kernel* `derivative_gpu` recebe os arrays de entrada e saída, e cada *thread* avalia a função para seu respectivo ponto de forma independente. 

O domínio definido para os testes teve um tamanho total de $N = 10.000.000$ pontos, e os *blocks* do CUDA foram configurados com um tamanho de 256 *threads*.

## 3. Resultados e Discussão
Os resultados foram obtidos executando o código no cluster NPAD, utilizando a partição de GPU `gpu-8-v100`. 

A saída produzida pela execução foi:
```text
CPU Execution Time: 0.175199 seconds
GPU Execution Time: 0.252764 seconds
Max error between CPU and GPU: 5.960464e-03
Speedup: 0.693133x
```

**Análise dos Resultados:**
- **Corretude:** O erro máximo registrado entre o cálculo na CPU e o cálculo na GPU foi de aproximadamente `0.00596`. Esse pequeno desvio ocorre devido a precisão de ponto flutuante simples (`float`) e características intrínsecas das operações matemáticas na arquitetura da GPU comparadas as da CPU.
- **Desempenho:** A execução na GPU (apenas o tempo de processamento do kernel) obteve um tempo ligeiramente superior ao tempo sequencial da CPU. Isso se deve, possivelmente, à simplicidade das operações (apenas duas chamadas de `sinf` e aritmética básica) comparadas com o *overhead* de *scheduling* dos blocos na GPU para uma carga computacional não tão intensa para cada *thread*. Em funções mais densas computacionalmente, o paralelismo de hardware da GPU apresentaria ganhos expressivos.

## 4. Conclusão
A paralelização do cálculo de derivadas numéricas com o método de diferenças finitas usando CUDA é direta, já que o problema possui características de *Data Parallelism* (Paralelismo de Dados). Embora neste cenário inicial os tempos de CPU tenham sido melhores em função de overheads do dispositivo e simplicidade do Kernel, a estrutura montada viabiliza a execução massivamente paralela para problemas complexos e multi-dimensionais.

---

## Apêndice: Código Fonte (`tarefa.cu`)

```c
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cuda_runtime.h>
#include <sys/time.h>

#define N 10000000 // Number of points
#define BLOCK_SIZE 256

// Function to calculate execution time
double get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec * 1e-6;
}

// Function whose derivative we want to calculate
__device__ __host__ float f(float x) {
    return sinf(x); // Example: f(x) = sin(x)
}

// CUDA Kernel for calculating derivative using central difference
__global__ void derivative_gpu(float *d_out, float *d_in, float h, int n) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < n) {
        float x = d_in[i];
        d_out[i] = (f(x + h) - f(x - h)) / (2.0f * h);
    }
}

// CPU function for calculating derivative
void derivative_cpu(float *out, float *in, float h, int n) {
    for (int i = 0; i < n; i++) {
        float x = in[i];
        out[i] = (f(x + h) - f(x - h)) / (2.0f * h);
    }
}

int main() {
    float *h_in, *h_out_cpu, *h_out_gpu;
    float *d_in, *d_out;
    float h = 1e-5; // Step size for finite differences
    size_t size = N * sizeof(float);

    // Allocate host memory
    h_in = (float *)malloc(size);
    h_out_cpu = (float *)malloc(size);
    h_out_gpu = (float *)malloc(size);

    if (h_in == NULL || h_out_cpu == NULL || h_out_gpu == NULL) {
        fprintf(stderr, "Failed to allocate host vectors!\n");
        exit(EXIT_FAILURE);
    }

    // Initialize domain (e.g., from 0 to 2*pi)
    float x_start = 0.0f;
    float x_end = 2.0f * (float)M_PI;
    float step = (x_end - x_start) / (N - 1);
    for (int i = 0; i < N; i++) {
        h_in[i] = x_start + i * step;
    }

    // --- CPU Execution ---
    double cpu_start = get_time();
    derivative_cpu(h_out_cpu, h_in, h, N);
    double cpu_end = get_time();
    double cpu_time = cpu_end - cpu_start;
    printf("CPU Execution Time: %f seconds\n", cpu_time);

    // --- GPU Execution ---
    // Allocate device memory
    cudaError_t err;
    err = cudaMalloc((void **)&d_in, size);
    if (err != cudaSuccess) {
        fprintf(stderr, "Failed to allocate device vector d_in (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }
    err = cudaMalloc((void **)&d_out, size);
    if (err != cudaSuccess) {
        fprintf(stderr, "Failed to allocate device vector d_out (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    // Copy data from host to device
    cudaMemcpy(d_in, h_in, size, cudaMemcpyHostToDevice);

    // Define block and grid dimensions
    int num_blocks = (N + BLOCK_SIZE - 1) / BLOCK_SIZE;

    // Synchronize and time GPU execution
    cudaDeviceSynchronize();
    double gpu_start = get_time();
    
    // Launch kernel
    derivative_gpu<<<num_blocks, BLOCK_SIZE>>>(d_out, d_in, h, N);
    
    err = cudaGetLastError();
    if (err != cudaSuccess) {
        fprintf(stderr, "Failed to launch derivative kernel (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    cudaDeviceSynchronize(); // Wait for GPU to finish
    double gpu_end = get_time();
    double gpu_time = gpu_end - gpu_start;
    printf("GPU Execution Time: %f seconds\n", gpu_time);

    // Copy result back to host
    cudaMemcpy(h_out_gpu, d_out, size, cudaMemcpyDeviceToHost);

    // --- Verify Results ---
    float max_error = 0.0f;
    for (int i = 0; i < N; i++) {
        float error = fabsf(h_out_cpu[i] - h_out_gpu[i]);
        if (error > max_error) {
            max_error = error;
        }
    }
    printf("Max error between CPU and GPU: %e\n", max_error);
    printf("Speedup: %fx\n", cpu_time / gpu_time);

    // Free memory
    free(h_in);
    free(h_out_cpu);
    free(h_out_gpu);
    cudaFree(d_in);
    cudaFree(d_out);

    return 0;
}
```
