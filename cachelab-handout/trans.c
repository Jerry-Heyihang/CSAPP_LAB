/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

#define min(x, y) ((x < y) ? (x) : (y))

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    if (M == 32) {
        int i, j, k, l;
        int temp[8];

        for (i = 0; i < N; i += 8) {
            for (j = 0; j < M; j += 8) {
                for (k = 0; k < 8; k++) {
                    for (l = 0; l < 8; l++) {
                        temp[l] = A[i + k][j + l];
                    }
                    for (l = 0; l < 8; l++) {
                        B[j + l][i + k] = temp[l];
                    }
                }
            }
        }
    } else if (M == 64) {
        int i, j, k, l, m; // n;
        int temp[8];
        for (i = 0; i < N; i += 8) {
            for (j = 0; j < M; j += 8) {
                for (k = i; k < i + 4; k++) {
                    for (l = 0; l < 8; l++) {
                        temp[l] = A[k][j + l];
                    }
                    for (l = 0; l < 4; l++) {
                        B[j + l][k] = temp[l];
                    }       
                    for (l = 0; l < 4; l++) {
                        B[j + l][k + 4] = temp[7 - l];
                    }
                }
                for (l = 0; l < 4; l++) {
                    for (m = 0; m < 4; m++) {
                        temp[m] = A[i + m + 4][j + 3 - l];
                    }
                    for (m = 0; m < 4; m++) {
                        temp[m + 4] = A[i + m + 4][j + l + 4];
                    }
                    for (m = 0; m < 4; m++) {
                        B[j + l + 4][i + m] = B[j + 3 - l][i + m + 4];
                        B[j + l + 4][i + m + 4] = temp[m + 4];
                    }
                    for (m = 0; m < 4; m++) {
                        B[j + 3 - l][i + m + 4] = temp[m];
                    }
                }   
            }
        }
    } else if (M == 61) {
        int i, j, k, l;
        for (i = 0; i < N; i += 17)
        {
            for (j = 0; j < M; j += 4)
            {
                for (k = i; k < min(i + 17, N); k++)
                {
                    for (l = j; l < min(j + 4, M); l++)
                    {
                        B[l][k] = A[k][l];
                    }
                }
            }
        }
    }

    return;
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    
    return;
}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

