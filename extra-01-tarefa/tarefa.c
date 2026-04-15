#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <unistd.h>

#define BUFFER_SIZE 5

int buffer[BUFFER_SIZE];
int count = 0;

int main() {
    #pragma omp parallel num_threads(2)
    {
        int thread_id = omp_get_thread_num();

        while(1) {
            if (thread_id == 0) { 
				// Thread Produtora
                int item = rand() % 100;

                #pragma omp critical
                {
                    if (count < BUFFER_SIZE) {
                        buffer[count] = item;
                        count++;
                        printf("Produtor: inseriu %d | Total: %d\n", item, count);
                    }
                }
                usleep(500000);
            } 
            if(thread_id == 1) {
                int item;
				// Thread Consumidora
                #pragma omp critical
                {
                    if (count > 0) {
                        count--;
                        item = buffer[count];
                        printf("Consumidor: removeu %d | Total: %d\n", item, count);
                    }
                }
                usleep(500000);
            }
        }
    }

    return 0;
}