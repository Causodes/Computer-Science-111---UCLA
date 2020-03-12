/**
Name: Tian Ye
Email: tianyesh@gmail.com
UID: 704931660
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>

/**
References:
	http://www.manpagez.com/man/3/clock_gettime/
	https://computing.llnl.gov/tutorials/pthreads/
	https://gcc.gnu.org/onlinedocs/gcc/_005f_005fsync-Builtins.html
	https://linux.die.net/man/3/pthread_mutex_unlock
*/

/**
Global Variables:
*/
int num_threads = 1;
int num_iterations = 1;
long long counter = 0;
pthread_t *threads;
int t;
int opt_yield = 0;
char lock = 'n';
int spin_lock = 0;
pthread_mutex_t mutex;

/**
Basic Add Routine:
*/
void add(long long *pointer, long long value) {
	long long sum = *pointer + value;
	if (opt_yield) {
		sched_yield();
	}
	*pointer = sum;
}

/**
Thread Calculation Handling:
*/
void *thread_sum(void *pointer) {
	long long compare_swap_previous, compare_swap_new;
	int i;
	for(i = 0; i < num_iterations; i++) {
		switch(lock) {
			// No lock
			case 'n':
				add((long long *) pointer, 1);
				add((long long *) pointer, -1);
				break;
			// Spin lock
			case 's':
				while(__sync_lock_test_and_set(&spin_lock, 1));
				add((long long *) pointer, 1);
				__sync_lock_release(&spin_lock);
				while(__sync_lock_test_and_set(&spin_lock, 1));
				add((long long *) pointer, -1);
				__sync_lock_release(&spin_lock);
				break;
			// Mutex lock
			case 'm':
				pthread_mutex_lock(&mutex);
				add((long long *) pointer, 1);
				pthread_mutex_unlock(&mutex);
				pthread_mutex_lock(&mutex);
				add((long long *) pointer, -1);
				pthread_mutex_unlock(&mutex);
				break;
			// Compare and swap lock
			case 'c':
				do {
					compare_swap_previous = *((long long *) pointer);
					compare_swap_new = compare_swap_previous + 1;
					if (opt_yield) {
						sched_yield();
					}
				} while (__sync_val_compare_and_swap((long long *) pointer, compare_swap_previous, compare_swap_new) != compare_swap_previous);
				do {
					compare_swap_previous = *((long long *) pointer);
					compare_swap_new = compare_swap_previous - 1;
					if (opt_yield) {
						sched_yield();
					}
				} while (__sync_val_compare_and_swap((long long *) pointer, compare_swap_previous, compare_swap_new) != compare_swap_previous);
				break;
			default:
				fprintf(stderr, "Error, unrecognized sync argument.\n");
				exit(1);
				break;
		}
	}
	return NULL;
}


/**
Output Handling:
*/
void output(int thread_num, int iteration_num, struct timespec time_begin, struct timespec time_end, long long counter_total) {
	int num_operations = thread_num * iteration_num * 2;
	long long total_time = (time_end.tv_sec - time_begin.tv_sec) * 1000000000 + (time_end.tv_nsec - time_begin.tv_nsec);
	long long avg_time = total_time/num_operations;
	char str_beginning[11];
	char str_end[5];
	if(opt_yield) {
		strcpy(str_beginning, "add-yield-");
	}
	else {
		strcpy(str_beginning, "add-");
	}
	if(lock == 'n') {
		strcpy(str_end, "none");
	}
	else {
		str_end[0] = lock;
	}
	printf("%s%s,%d,%d,%d,%lld,%lld,%lld\n", str_beginning, str_end, thread_num, iteration_num, num_operations, total_time, avg_time, counter_total);
}

/**
Main Program Routine:
*/
int main(int argc, char **argv) {
	
	// Handle execution argument
	int c = 0;
	static struct option long_options[] = {
		{"threads",		required_argument,	0,	't'},
		{"iterations",	required_argument, 	0,	'i'},
		{"yield",		no_argument,		0,	'y'},
		{"sync",		required_argument,	0,	's'}
	};
	
	while((c = getopt_long(argc, argv, "t:i:ys:", long_options, NULL)) != -1) {
		switch(c) {
			case 't':
				num_threads = atoi(optarg);
				break;
			case 'i':
				num_iterations = atoi(optarg);
				break;
			case 'y':
				opt_yield = 1;
				break;
			case 's':
				if(strlen(optarg) == 1) {
					switch(optarg[0]) {
						case 's':
						case 'm':
						case 'c':
							lock = optarg[0];
							break;
						default:
							fprintf(stderr, "Error, unrecognized sync argument.\n");
							exit(1);
							break;
					}
				}
				break;
			default:
				// TODO: Update Error Message
				fprintf(stderr, "Incorrect Argument Usage, Correct Usage: ./lab1b-client --port=port_number [--log=log_file] [--compress]\n");
				exit(1);
				break;
		}
	}
	
	// Notes starting time for run
	struct timespec begin;
	if(clock_gettime(CLOCK_MONOTONIC, &begin) == -1) {
		fprintf(stderr, "Error, invalid clock start time.\n");
		exit(1);
	}
	
	// Starts specified number of threads
	threads = malloc(num_threads * sizeof(pthread_t));
	
	for(t = 0; t < num_threads; t++) {
		int rc = pthread_create(&threads[t], NULL, thread_sum, &counter);
		// Error checking
		if(rc) {
			fprintf(stderr, "Error, creating thread number %d failed.\n", t);
			exit(1);
		}
	}
	
	// Joins threads
	for(t = 0; t < num_threads; t++) {
		int rc = pthread_join(threads[t], NULL);
		// Error checking
		if(rc) {
			fprintf(stderr, "Error, joining thread number %d failed.\n", t);
			free(threads);
			exit(1);
		}
	}
		
	// Cleanup
	free(threads);
	if(lock == 'm') {
		pthread_mutex_destroy(&mutex);
	}
	
	// Notes ending time for run
	struct timespec end;
		if(clock_gettime(CLOCK_MONOTONIC, &end) == -1) {
		fprintf(stderr, "Error, invalid clock end time.\n");
		exit(1);
	}
	
	// Output handling
	output(num_threads, num_iterations, begin, end, counter);
	
	return 0;
}