/**
 * This file implements parallel mergesort.
 */

#include <stdio.h>
#include <string.h> /* for memcpy */
#include <stdlib.h> /* for malloc */
#include <pthread.h>
#include "mergesort.h"

/* this function will be called by mergesort() and also by parallel_mergesort(). */
void merge(int leftstart, int leftend, int rightstart, int rightend){
    int i = leftstart;
    int j = rightstart;
    int k = leftstart;

    int size = rightend - leftstart + 1;

    // Merge the two sorted halves
    while (i <= leftend && j <= rightend) {
        if (A[i] <= A[j]) {
            B[k] = A[i];
            i++;
        } else {
            B[k] = A[j];
            j++;
        }
        k++;
    }

    // Copy remaining elements from left half
    while (i <= leftend) {
        B[k] = A[i];
        i++;
        k++;
    }

    // Copy remaining elements from right half
    while (j <= rightend) {
        B[k] = A[j];
        j++;
        k++;
    }

    // Copy B array back into A
    for (i = leftstart; i <= rightend; i++) {
        A[i] = B[i];
    }
}






/* this function will be called by parallel_mergesort() as its base case. */
void my_mergesort(int left, int right){
    if (left >= right) { return; }

    int middle = (left + right) / 2;

    my_mergesort(left, middle);
    my_mergesort(middle + 1, right);

    merge(left, middle, middle + 1, right);
}









/* this function will be called by the testing program. */
void * parallel_mergesort(void *arg){

    struct argument *args = (struct argument *)arg;

    int left = args->left;
    int right = args->right;
    int level = args->level;
    pthread_t thread1, thread2;

    if (level < cutoff && left < right) {
        int middle = (left + right) / 2;

        // left half, create a new thread to sort it
        struct argument *arg1=buildArgs(left,  middle, level+1);
        pthread_create(&thread1, NULL, parallel_mergesort, arg1);

        // right half, create a new thread to sort it
        struct argument *arg2=buildArgs(middle+1,  right, level+1);
        pthread_create(&thread2, NULL, parallel_mergesort, arg2);

        // wait for both threads to finish
        pthread_join(thread1, NULL);
        pthread_join(thread2, NULL);

        // free the arguments
        free(arg1);
        free(arg2);

        // merge the two halves
        merge(left, middle,middle+1,right);
    } else {
        my_mergesort(left, right);
    }

    return NULL;
}



/* we build the argument for the parallel_mergesort function. */
struct argument *buildArgs(int left, int right, int level){
    struct argument *args = malloc(sizeof(struct argument));

    // check if malloc failed
    if (args == NULL) {
        fprintf(stderr, "Error: malloc failed in buildArgs\n");
        exit(EXIT_FAILURE);
    }
    
    args->left = left;
    args->right = right;
    args->level = level;

    return args;
}

