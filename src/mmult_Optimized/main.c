/* Standard C includes */
#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sched.h>
#include <stdbool.h>
#include <inttypes.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

/* Include all implementations declarations */
#include "impl/naive.h"
#include "impl/opt.h"  
#include "impl/simd.h" 
#include "impl/mimd.h"  
/* Include common headers */
#include "common/types.h"
#include "common/macros.h"

/* Include application-specific headers */
#include "include/types.h"

/* Helper function to print a matrix */
void print_matrix(const char* name, float* matrix, size_t rows, size_t cols) {
    printf("%s:\n", name);
    for (size_t i = 0; i < rows; i++) {
        for (size_t j = 0; j < cols; j++) {
            printf("%.2f ", matrix[i * cols + j]);
        }
        printf("\n");
    }
    printf("\n");
}

/* Helper function to create the Result directory */
void create_result_directory() {
    struct stat st = {0};
    if (stat("Result", &st) == -1) {
        if (mkdir("Result", 0700) == 0) {
            printf("Result directory created successfully.\n");
        } else {
            perror("Error creating Result directory");
        }
    }
}

/* Helper function to export a matrix to a CSV file in the Result directory */
void export_matrix_to_csv(const char* filename, float* matrix, size_t rows, size_t cols) {
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "Result/%s", filename);

    FILE* file = fopen(filepath, "w");
    if (!file) {
        fprintf(stderr, "Error opening file %s for writing.\n", filepath);
        return;
    }

    for (size_t i = 0; i < rows; i++) {
        for (size_t j = 0; j < cols; j++) {
            fprintf(file, "%.6f", matrix[i * cols + j]);
            if (j < cols - 1) {
                fprintf(file, ",");
            }
        }
        fprintf(file, "\n");
    }

    fclose(file);
}

int main(int argc, char** argv) {
    /* Set the buffer for printf to NULL */
    setbuf(stdout, NULL);
    /* Default settings */
    void* (*impl)(void* args) = NULL;
    const char* impl_str = NULL;
    bool run_both = false;
    /* Parse command-line arguments */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--impl") == 0) {
            assert(++i < argc);
            if (strcmp(argv[i], "naive") == 0) {
                impl = impl_scalar_naive;
                impl_str = "naive";
            } else if (strcmp(argv[i], "opt") == 0) {
                impl = impl_scalar_opt;
                impl_str = "opt";
            }else if (strcmp(argv[i], "simd") == 0) {
                impl = impl_simd;
                impl_str = "simd";
            }else if (strcmp(argv[i], "mimd") == 0) {
                impl = impl_mimd;
                impl_str = "mimd";
            }  else if (strcmp(argv[i], "all") == 0) {
                run_both = true;
            } else {
                fprintf(stderr, "Unknown implementation: %s\n", argv[i]);
                exit(1);
            }
            continue;
        }
    }
    if (impl == NULL && !run_both) {
        fprintf(stderr, "Usage: %s -i {naive|opt|simd|mimd|all}\n", argv[0]);
        exit(1);
    }
    /* Create the Result directory */
    create_result_directory();
    /* Prompt the user for matrix dimensions */
    size_t rows_A, cols_A, rows_B, cols_B;
    printf("Enter the number of rows for Matrix A: ");
    scanf("%zu", &rows_A);
    printf("Enter the number of columns for Matrix A: ");
    scanf("%zu", &cols_A);
    printf("Enter the number of rows for Matrix B: ");
    scanf("%zu", &rows_B);
    /* Ensure cols_A == rows_B */
    while (cols_A != rows_B) {
        printf("Number of columns for Matrix A must equal the number of rows for Matrix B.\n");
        printf("Enter the number of rows for Matrix B: ");
        scanf("%zu", &rows_B);
    }
    printf("Enter the number of columns for Matrix B: ");
    scanf("%zu", &cols_B);

    /* Allocate matrices */
    srand((unsigned int)time(NULL)); 

    float* A = malloc(rows_A * cols_A * sizeof(float));
    float* B = malloc(rows_B * cols_B * sizeof(float));
    float* R_naive = malloc(rows_A * cols_B * sizeof(float));
    float* R_opt = malloc(rows_A * cols_B * sizeof(float));

    float* R_vec = malloc(rows_A * cols_B * sizeof(float));
    float* R_para = malloc(rows_A * cols_B * sizeof(float));
    float* R_simd = malloc(rows_A * cols_B * sizeof(float));
    float* R_mimd = malloc(rows_A * cols_B * sizeof(float));
    if (!A || !B || !R_naive || !R_opt|| !R_mimd|| !R_simd) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(1);
    }

    /* Initialize input matrices */
    for (size_t i = 0; i < rows_A * cols_A; i++) {
        A[i] = (float)(rand() % 10);
    }

    for (size_t i = 0; i < rows_B * cols_B; i++) {
        B[i] = (float)(rand() % 10);
    }

    /* Print input matrices */
   /* print_matrix("Matrix A", A, rows_A, cols_A);*/
   /* print_matrix("Matrix B", B, rows_B, cols_B);*/

    /* Export input matrices */
    export_matrix_to_csv("matrix_A.csv", A, rows_A, cols_A);
    export_matrix_to_csv("matrix_B.csv", B, rows_B, cols_B);

    double naive_time = 0.0, opt_time = 0.0, vec_time = 0.0, para_time = 0.0, simd_time = 0.0, mimd_time = 0.0;// Declare timing variables
    /* Run naive implementation */
    if (run_both || impl == impl_scalar_naive) {
        args_t args_naive = { .input = malloc((rows_A * cols_A + rows_B * cols_B) * sizeof(float)), .output = R_naive, .size = rows_A };
        memcpy(args_naive.input, A, rows_A * cols_A * sizeof(float));
        memcpy((float*)args_naive.input + rows_A * cols_A, B, rows_B * cols_B * sizeof(float));

        clock_t start_naive = clock();
        impl_scalar_naive(&args_naive);
        clock_t end_naive = clock();

        naive_time = (double)(end_naive - start_naive) / CLOCKS_PER_SEC;
        printf("Naive Implementation Runtime: %.6f seconds\n", naive_time);
        /* print_matrix("Result Matrix R (Naive)", R_naive, rows_A, cols_B);*/
        export_matrix_to_csv("result_naive.csv", R_naive, rows_A, cols_B);

        free(args_naive.input);
    }

    /* Run optimized implementation */
    if (run_both || impl == impl_scalar_opt) {
        args_t args_opt = { .input = malloc((rows_A * cols_A + rows_B * cols_B) * sizeof(float)), .output = R_opt, .size = rows_A };
        memcpy(args_opt.input, A, rows_A * cols_A * sizeof(float));
        memcpy((float*)args_opt.input + rows_A * cols_A, B, rows_B * cols_B * sizeof(float));

        clock_t start_opt = clock();
        impl_scalar_opt(&args_opt);
        clock_t end_opt = clock();

        opt_time = (double)(end_opt - start_opt) / CLOCKS_PER_SEC;
        printf("Optimized Implementation Runtime: %.6f seconds\n", opt_time);
       /* print_matrix("Result Matrix R (Optimized)", R_opt, rows_A, cols_B); */
        export_matrix_to_csv("result_opt.csv", R_opt, rows_A, cols_B);

        free(args_opt.input);
    }
        /* Run simd implementation */
    if (run_both || impl == impl_simd) {
        args_t args_simd = { .input = malloc((rows_A * cols_A + rows_B * cols_B) * sizeof(float)), .output = R_simd, .size = rows_A };
        memcpy(args_simd.input, A, rows_A * cols_A * sizeof(float));
        memcpy((float*)args_simd.input + rows_A * cols_A, B, rows_B * cols_B * sizeof(float));

        clock_t start_simd = clock();
        impl_simd(&args_simd);
        clock_t end_simd = clock();

        simd_time = (double)(end_simd - start_simd) / CLOCKS_PER_SEC;
        printf("SIMD Implementation Runtime: %.6f seconds\n", simd_time);
        /* print_matrix("Result Matrix R (SIMD)", R_simd, rows_A, cols_B);*/
        export_matrix_to_csv("result_simd.csv", R_simd, rows_A, cols_B);

        free(args_simd.input);
    }

        /* Run mimd implementation */
    if (run_both || impl == impl_mimd) {
        args_t args_mimd = { .input = malloc((rows_A * cols_A + rows_B * cols_B) * sizeof(float)), .output = R_mimd, .size = rows_A };
        memcpy(args_mimd.input, A, rows_A * cols_A * sizeof(float));
        memcpy((float*)args_mimd.input + rows_A * cols_A, B, rows_B * cols_B * sizeof(float));

        clock_t start_mimd = clock();
        impl_mimd(&args_mimd);
        clock_t end_mimd = clock();

        mimd_time = (double)(end_mimd - start_mimd) / CLOCKS_PER_SEC;
        printf("MIMD Implementation Runtime: %.6f seconds\n", mimd_time);
        /* print_matrix("Result Matrix R (MIMD)", R_mimd, rows_A, cols_B);*/
        export_matrix_to_csv("result_mimd.csv", R_mimd, rows_A, cols_B);

        free(args_mimd.input);
    }

  

      /* Calculate and print speedup */
    if (naive_time > 0) {
        if (opt_time > 0) {
            printf("Speedup (Optimized vs Naive): %.2fx\n", naive_time / opt_time);
        }
        if (simd_time > 0) {
            printf("Speedup (SIMD vs Naive): %.2fx\n", naive_time / simd_time);
        }
        if (mimd_time > 0) {
            printf("Speedup (MIMD vs Naive): %.2fx\n", naive_time / mimd_time);
        }
    }


    /* Free memory */
    free(A);
    free(B);
    free(R_naive);
    free(R_opt);

    return 0;
}
