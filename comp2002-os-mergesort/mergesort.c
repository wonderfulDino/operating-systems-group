/**
 * This file implements parallel mergesort.
 */

#include <stdio.h>
#include <string.h> /* for memcpy */
#include <stdlib.h> /* for malloc */
#include "mergesort.h"

/* this function will be called by mergesort() and also by parallel_mergesort(). */
void merge(int leftstart, int leftend, int rightstart, int rightend){
}

/* this function will be called by parallel_mergesort() as its base case. */
void my_mergesort(int left, int right){
}

/* this function will be called by the testing program. */
void * parallel_mergesort(void *arg){
    struct argument *args = (struct argument *)arg;

    //int left = args->left;
    int right = args->right;
    //int level = args->level;

    int n = right+1;

	int i, j, minIndex, temp;

    for (i = 0; i < n - 1; i++) {
        minIndex = i;

        for (j = i + 1; j < n; j++) {
            if (A[j] < A[minIndex]) {
                minIndex = j;
            }
        }

        temp = A[i];
        A[i] = A[minIndex];
        A[minIndex] = temp;
    }

    free(args);

    return NULL;
}

/* we build the argument for the parallel_mergesort function. */
struct argument *buildArgs(int left, int right, int level){
    struct argument *args = malloc(sizeof(struct argument));

    args->left = left;
    args->right = right;
    args->level = level;

    return args;
}

