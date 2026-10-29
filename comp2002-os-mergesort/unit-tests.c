#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/times.h> /* for times system call */
#include <sys/time.h>  /* for gettimeofday system call */
#include <unistd.h>
#include <error.h>     /* On MacOS you won't need this line */
#include "mergesort.h"
#include <stdbool.h>

/* the number of levels of threads, specified by the user */
int cutoff = 0;

/* the array we want to sort */
int *A;

/* when merge, we need a temp array (named B) to temporarily store elements of the original array */
int *B;

int successes_n = 0;
int tests_n = 0;
int tests_all = 0;
int successes_all = 0;

// function prototypes
int check_if_sorted(int A[], int n);
void generate_random_array(int A[], int n, int seed);
double getMilliSeconds(void);

#define COLOUR "\x1b[34m"
#define RESET "\x1b[0m"
#define GREY "\x1b[38;5;244m"
#define RED "\x1b[31m"

/*
---------------------------------------------------------------------------
clock_t times(struct tms *buffer);
times() fills the structure pointed to by buffer with
time-accounting information.  The structure defined in
<sys/times.h> is as follows:
struct tms {
    clock_t tms_utime;       user time
    clock_t tms_stime;       system time
    clock_t tms_cutime;      user time, children
    clock_t tms_cstime;      system time, children
The time is given in units of 1/CLK_TCK seconds where the
value of CLK_TCK can be determined using the sysconf() function
with the agrgument _SC_CLK_TCK.
---------------------------------------------------------------------------
*/


/* we do not really use this function anymore, but somehow we still keep it here. */
float report_cpu_time(void)
{
	struct tms buffer;
	float cputime;

	times(&buffer);
	cputime = (buffer.tms_utime)/ (float) sysconf(_SC_CLK_TCK);
	return (cputime);
}

/* we do not really use this function anymore, but somehow we still keep it here. */
float report_sys_time(void)
{
	struct tms buffer;
	float systime;

	times(&buffer);
	systime = (buffer.tms_stime)/ (float) sysconf(_SC_CLK_TCK);
	return (systime);
}

double getMilliSeconds(void)
{
    struct timeval now;
    gettimeofday(&now, (struct timezone *)0);
    return (double) now.tv_sec*1000.0 + now.tv_usec/1000.0;
}


/*
 * generate_random_array(int A[], int n, int seed):
 *
 * description: Generate random integers in the range [0,RANGE]
 *              and store in A[0..n-1]
 */

#define RANGE 1000000

void generate_random_array(int A[], int n, int seed)
{
	int i;
	srandom(seed);
	for (i=0; i<n; i++)
        	A[i] = random()%RANGE;
}


/*
 * check_if_sorted(int A[], int n):
 *
 * description: returns TRUE if A[1..n] are sorted in nondecreasing order
 *              otherwise returns FALSE
 */

int check_if_sorted(int A[], int n)
{
	int i;
	for (i=0; i<n-1; i++) {
		if (A[i] > A[i+1]) {
			return FALSE;
		}
	}
	return TRUE;
}

/* this function prints A, but we only print the first 100 elements */
void printA(void){
	int i;
	printf("Array A:");
	/* FIXME: we assume the size of A is at least 100, and this will fail if it is lower than 100 */
	for(i=0;i<100;i++){
		printf(" %d",A[i]);
	}
	printf("\n");
}

/* this function prints B, but we only print the first 100 elements */
void printB(void){
	int i;
	printf("Array B:");
	/* FIXME: we assume the size of B is at least 100, and this will fail if it is lower than 100 */
	for(i=0;i<100;i++){
		printf(" %d",B[i]);
	}
	printf("\n");
}

void go_back_line(void) {
	printf("\33[1A\33[2K");
	printf("\r");
}

double test(int _n, int _cutoff, int _seed) {
	double start_time;
	double sorting_time;

	// values to vary
	int n = _n;
	cutoff = _cutoff; 
	int seed = _seed;

	printf(GREY "Sorting " COLOUR "%d" GREY " elements..."RESET"\n", n);

	// setting up variables
	A = (int *) malloc(sizeof(int) * n);
	B = (int *) malloc(sizeof(int) * n);

	// generate random input
	generate_random_array(A, n, seed);

	// sort the input
	start_time = getMilliSeconds();
	struct argument *arg=buildArgs(0, n-1, 0);
	parallel_mergesort(arg);
	sorting_time = getMilliSeconds() - start_time;

	tests_n ++;
	tests_all ++;

	go_back_line();

	// check the sorting results
	if (check_if_sorted(A,n)) {
		successes_n ++;
		successes_all ++;
		printf(GREY "Sorting " COLOUR "%d" GREY " elements took " COLOUR "%4.2lf" GREY " seconds."RESET"\n", n,  sorting_time/1000.0);
	} else {
		printf("sorting failed!!!!\n");
		free(arg);
		free(B);
		free(A);
		exit(EXIT_FAILURE);
	}

	/* Now we can free the memory for the two arrays, as well as free memory for arg. */
	free(arg);
	free(B);
	free(A);

	return sorting_time/1000.0;
}

// Unit test for buildArgs()
void test_buildArgs(int left, int right, int level) {

	struct argument *args = buildArgs(left, right, level);

	bool pass = true;
	// Check if NULL
	if (args == NULL) { printf(RED"ERROR: argument returned as NULL."RESET"\n"); pass = false; }
	// Check if values are assigned.
	if (args->left != left || args->right != right || args->level != level) { printf(RED"ERROR: argument formed incorrectly."RESET"\n"); pass = false; }

	if (pass) {
		successes_n ++;
		successes_all ++;
		printf(GREY"Test buildArgs(%d, %d, %d)"COLOUR" Passed."RESET"\n", left, right, level);
	}

	tests_n ++;
	tests_all ++;

	free(args);
}

void test_mergesort(int n) {

	// setting up variables
	A = (int *) malloc(sizeof(int) * n);
	B = (int *) malloc(sizeof(int) * n);

	// generate random input
    my_mergesort(0, n-1);

	tests_n ++;
	tests_all ++;

    if (check_if_sorted(A, n)) {
		successes_n ++;
		successes_all ++;
		printf(GREY"Test my_mergesort(0, %d)"COLOUR" Passed."RESET"\n", n-1);
	} else {
		printf(RED"Test my_mergesort(0, %d) Failed."RESET"\n", n-1);
	}

	free(A);
	free(B);
}

void section_start(const char *text) {
    printf("## %s ##\n", text);
    successes_n = 0;
    tests_n = 0;
}

void section_end(void) {
	if (successes_n != tests_n) { 
		printf(RED"%d / %d tests ran successfully."RESET" \n\n", successes_n, tests_n);
	} else {
    	printf(RESET"%d / %d tests ran successfully. \n\n", successes_n, tests_n);
	}
}

int main(void) {

	printf("\n");

	int i;

	// Tests for buildArgs()
	printf("## Unit Test buildArgs() ##\n\n");

	section_start("buildArgs() RANDOM VALUES");
	for(i = 0; i < 10; i++) {test_buildArgs(random()%RANGE,random()%RANGE,random()%RANGE);}
	section_end();

	section_start("buildArgs() ZEROES");
	test_buildArgs(0,0,0);
	section_end();

	section_start("buildArgs() NEGATIVE VALUES");
	for(i = 0; i < 10; i++) {test_buildArgs(-random()%RANGE,-random()%RANGE,-random()%RANGE);}
	section_end();

	// Tests for my_mergesort()
	section_start("Unit Test my_mergesort()");
	for(i = 0; i < 10; i++) { test_mergesort(i); }
	for(i = 100; i < 1000; i+=100) { test_mergesort(i); }
	for(i = 1000; i < 10000; i+=1001) { test_mergesort(i); }
	for(i = 100000; i < 1000000; i+=100000) { test_mergesort(i); }
	section_end();

	int cutoff_i;

	for(cutoff_i=0; cutoff_i<5; cutoff_i++) {

		printf("\n## Cutoff: %d ##\n\n", cutoff_i);

		section_start("ARRAY OF ZERO");
		test(0, cutoff_i, 1);
		section_end();

		section_start("ARRAY OF SMALL SIZES");
		for(i = 1; i <= 10; i++) { test(i, cutoff_i, 1); }
		section_end();

		section_start("ARRAY OF MEDIUM SIZES + ODDS, EVENS");
		for(i = 100; i <= 1000; i+=101) { test(i, cutoff_i, 1); }
		section_end();

		section_start("ARRAY OF LARGE SIZES");
		for(i = 10000; i <= 100000; i+=10000) { test(i, cutoff_i, 1); }
		section_end();

		section_start("ARRAY OF VERY LARGE SIZES");
		for(i = 100000; i <= 1000000; i+=100000) { test(i, cutoff_i, 1); }
		section_end();

		/*section_start("ARRAY OF MASSIVE SIZES");
		for(i = 1000000; i <= 10000000; i+=1000000) { test(i, cutoff_i, 1); }
		section_end();*/

		/*section_start("ARRAY OF VERY MASSIVE SIZES");
		for(i = 10000000; i <= 100000000; i+=10000000) { test(i, cutoff_i, 1); }
		section_end();*/
	}

	section_start("TESTING THREADS");
	double time = 0;
	double time_initial = 0;
	double percentage = 0;

	int duplicates = 3;
	int elements = 100000000;

	int u;

	printf("# Thread cutoff: %d \n", 0);
	for(u=0; u<duplicates; u++) { time_initial += test(elements, 0, 1); }
	time_initial /= (double)duplicates;

	go_back_line();
	go_back_line();
	go_back_line();

	printf(COLOUR "%d" GREY " elements took "COLOUR"%4.2lf"GREY" seconds."RESET"\n" , elements,  time_initial);

	for(i = 1; i <= 10; i++) { 
		printf("# Thread cutoff: %d \n", i);
		time = 0;
		for(u=0; u<duplicates; u++) { time += test(elements, i, 1); }
		time /= (double)duplicates;
		percentage = 100*time / time_initial;

		go_back_line();
		go_back_line();
		go_back_line();

		printf(COLOUR"%d"GREY" elements took "COLOUR"%4.2lf"GREY" seconds (n=%d)."RESET"\n", elements,  time,duplicates);
		//printf("Time (average): %.2f seconds \n", time);
		printf(COLOUR"%.1lf%%"GREY" compared to no threading."RESET"\n", percentage);
		//printf(RESET);
	}
	section_end();

	printf("## CONCLUSION ##\n");
	printf("%d / %d tests ran successfully. \n\n", successes_all, tests_all);

	exit(EXIT_SUCCESS);
}
