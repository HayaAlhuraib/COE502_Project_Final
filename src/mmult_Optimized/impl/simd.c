
/* Standard C includes */
#include <immintrin.h>  // For AVX intrinsics

#include <stdlib.h>

/* Include common headers */
#include "common/macros.h"
#include "common/types.h"

/* Include application-specific headers */
#include "include/types.h"



/* SIMD Implementation */
void* impl_simd(void* args) {
    /* Extract arguments */
    args_t* arguments = (args_t*)args;
    size_t size = arguments->size;

    float* A = (float*)arguments->input;
    float* B = (float*)(arguments->input + size * size * sizeof(float));
    float* R = (float*)arguments->output;

    /* Perform matrix-matrix multiplication using AVX */
    for (size_t i = 0; i < size; i++) {
        for (size_t j = 0; j < size; j++) {
            __m256 sum = _mm256_setzero_ps();  // Initialize the sum vector to zero

            for (size_t k = 0; k < size; k += 8) {
                // Load 8 elements from A and B
                __m256 a_vec = _mm256_loadu_ps(&A[i * size + k]);
                __m256 b_vec = _mm256_loadu_ps(&B[k * size + j]);

                // Perform element-wise multiplication and accumulate the results
                sum = _mm256_fmadd_ps(a_vec, b_vec, sum);  // sum += a_vec * b_vec
            }

            // Sum the elements in the vector
            float temp[8];
            _mm256_storeu_ps(temp, sum);
            float total = temp[0] + temp[1] + temp[2] + temp[3] + temp[4] + temp[5] + temp[6] + temp[7];

            R[i * size + j] = total;
        }
    }

    return NULL;
}

