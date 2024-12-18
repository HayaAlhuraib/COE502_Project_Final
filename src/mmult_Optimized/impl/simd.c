

#include <stdlib.h>

/* Include common headers */
#include "common/macros.h"
#include "common/types.h"

/* Include application-specific headers */
#include "include/types.h"
#include <arm_neon.h>  // For ARM NEON intrinsics

/* SIMD Implementation using ARM NEON */
void* impl_simd(void* args) {
    /* Extract arguments */
    args_t* arguments = (args_t*)args;
    size_t size = arguments->size;

    float* A = (float*)arguments->input;
    float* B = (float*)(arguments->input + size * size * sizeof(float));
    float* R = (float*)arguments->output;

    /* Perform matrix-matrix multiplication using ARM NEON */
    for (size_t i = 0; i < size; i++) {
        for (size_t j = 0; j < size; j++) {
            float32x4_t sum_vec = vdupq_n_f32(0.0f);  // Initialize the sum vector to zero

            for (size_t k = 0; k < size; k += 4) {
                // Load 4 elements from A and B
                float32x4_t a_vec = vld1q_f32(&A[i * size + k]);
                float32x4_t b_vec = vld1q_f32(&B[k * size + j]);

                // Perform element-wise multiplication and accumulate the results
                sum_vec = vmlaq_f32(sum_vec, a_vec, b_vec);  // sum_vec += a_vec * b_vec
            }

            // Sum the elements in the vector
            float temp[4];
            vst1q_f32(temp, sum_vec);
            float total = temp[0] + temp[1] + temp[2] + temp[3];

            R[i * size + j] = total;
        }
    }

    return NULL;
}
