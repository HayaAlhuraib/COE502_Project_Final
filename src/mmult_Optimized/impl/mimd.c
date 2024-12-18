/* Standard C includes */
#include <stdlib.h>
#include <pthread.h>
#include "include/types.h"

/* Structure to pass data to each thread */
typedef struct {
    float* A;
    float* B;
    float* R;
    size_t size;
    size_t start_row;
    size_t end_row;
} thread_data_t;

/* Function executed by each thread */
void* mimd_worker(void* arg) {
    thread_data_t* data = (thread_data_t*)arg;
    float* A = data->A;
    float* B = data->B;
    float* R = data->R;
    size_t size = data->size;

    for (size_t i = data->start_row; i < data->end_row; i++) {
        for (size_t j = 0; j < size; j++) {
            float sum = 0.0f;
            for (size_t k = 0; k < size; k++) {
                sum += A[i * size + k] * B[k * size + j];
            }
            R[i * size + j] = sum;
        }
    }

    return NULL;
}

/* MIMD Implementation */
void* impl_mimd(void* args) {
    /* Extract arguments */
    args_t* arguments = (args_t*)args;
    size_t size = arguments->size;
    float* A = (float*)arguments->input;
    float* B = (float*)(arguments->input + size * size * sizeof(float));
    float* R = (float*)arguments->output;
    int nthreads = 4;

    pthread_t threads[nthreads];
    thread_data_t thread_data[nthreads];

    size_t rows_per_thread = size / nthreads;

    for (int t = 0; t < nthreads; t++) {
        thread_data[t].A = A;
        thread_data[t].B = B;
        thread_data[t].R = R;
        thread_data[t].size = size;
        thread_data[t].start_row = t * rows_per_thread;
        thread_data[t].end_row = (t == nthreads - 1) ? size : (t + 1) * rows_per_thread;

        pthread_create(&threads[t], NULL, mimd_worker, &thread_data[t]);
    }

    for (int t = 0; t < nthreads; t++) {
        pthread_join(threads[t], NULL);
    }

    return NULL;
}
