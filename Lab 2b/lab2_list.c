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
#include <signal.h>
#include "SortedList.h"

/**
References:
	http://www.manpagez.com/man/3/clock_gettime/
	https://computing.llnl.gov/tutorials/pthreads/
	https://gcc.gnu.org/onlinedocs/gcc/_005f_005fsync-Builtins.html
	https://linux.die.net/man/3/pthread_mutex_unlock
	http://www.cse.yorku.ca/~oz/hash.html
	https://linux.die.net/man/1/pprof
*/

/**
Global Variables:
*/
int num_threads = 1;
int num_iterations = 1;
int num_lists = 1;
pthread_t *threads;
int t;
int opt_yield;
unsigned int j;
char lock = 'n';
int *spin_lock_array;
pthread_mutex_t *mutex_array;
SortedList_t *list;
SortedListElement_t * element;
int list_length = 0;
char str_yield[5] = "none";
long long *total_time_array;

/**
Signal Handler Function:
*/
void signal_handler() {
  fprintf(stderr, "SIGNAL SEGFAULT\n");
  exit(1);
}

/**
Hash Function:
*/
unsigned long hash(const char *str) {
	unsigned long hash = 5381;
	int c;
	while ((c = *str++)) {
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	}
	return hash % num_lists;
}


/**
Node Delete Function:
*/
void delete_node(SortedListElement_t *node, int i, int index) {
	if((node = SortedList_lookup(&list[index], element[i].key))) {
		if(SortedList_delete(node)) {
			fprintf(stderr, "Error during deletion.\n");
			exit(2);
		}
	}
	else {
		fprintf(stderr, "Missing element %d.\n", i);
		exit(2);
	}
}


/**
Thread Execution:
*/
void *thread_exec(void *thread_counter) {
	struct timespec start, stop;
	int i;
	for(i = (*(int *)thread_counter); i < num_threads * num_iterations; i+=num_threads) {
		int index = hash(element[i].key);
		switch(lock) {
			case 'n':
				SortedList_insert(&list[index], &element[i]);
				break;
			case 'm':
				if(clock_gettime(CLOCK_MONOTONIC, &start) == -1) {
					fprintf(stderr, "Error, invalid clock start time.\n");
					exit(1);
				}
				pthread_mutex_lock(&mutex_array[index]);
				if(clock_gettime(CLOCK_MONOTONIC, &stop) == -1) {
					fprintf(stderr, "Error, invalid clock end time.\n");
					exit(1);
				}
				SortedList_insert(&list[index], &element[i]);
				pthread_mutex_unlock(&mutex_array[index]);
				total_time_array[(*(int *)thread_counter)] += (stop.tv_sec - start.tv_sec) * 1000000000 + (stop.tv_nsec - start.tv_nsec);
				break;
			case 's':
				if(clock_gettime(CLOCK_MONOTONIC, &start) == -1) {
					fprintf(stderr, "Error, invalid clock start time.\n");
					exit(1);
				}
				while (__sync_lock_test_and_set(&spin_lock_array[index], 1));
				if(clock_gettime(CLOCK_MONOTONIC, &stop) == -1) {
					fprintf(stderr, "Error, invalid clock end time.\n");
					exit(1);
				}
				SortedList_insert(&list[index], &element[i]);
				__sync_lock_release(&spin_lock_array[index]);
				total_time_array[(*(int *)thread_counter)] += (stop.tv_sec - start.tv_sec) * 1000000000 + (stop.tv_nsec - start.tv_nsec);
				break;
			default:
				fprintf(stderr, "Error, unrecognized sync argument.\n");
				exit(1);
				break;
		}
	}
	
	SortedListElement_t *node = NULL;
	for(i = (*(int *)thread_counter); i < num_threads * num_iterations; i+=num_threads) {
		int index = hash(element[i].key);
		switch(lock) {
			case 'n':
				delete_node(node, i, index);
				break;
			case 'm':
				if(clock_gettime(CLOCK_MONOTONIC, &start) == -1) {
					fprintf(stderr, "Error, invalid clock start time.\n");
					exit(1);
				}
				pthread_mutex_lock(&mutex_array[index]);
				if(clock_gettime(CLOCK_MONOTONIC, &stop) == -1) {
					fprintf(stderr, "Error, invalid clock end time.\n");
					exit(1);
				}
				delete_node(node, i, index);
				pthread_mutex_unlock(&mutex_array[index]);
				total_time_array[(*(int *)thread_counter)] += (stop.tv_sec - start.tv_sec) * 1000000000 + (stop.tv_nsec - start.tv_nsec);
				break;
			case 's':
				if(clock_gettime(CLOCK_MONOTONIC, &start) == -1) {
					fprintf(stderr, "Error, invalid clock start time.\n");
					exit(1);
				}
				while (__sync_lock_test_and_set(&spin_lock_array[index], 1));
				if(clock_gettime(CLOCK_MONOTONIC, &stop) == -1) {
					fprintf(stderr, "Error, invalid clock end time.\n");
					exit(1);
				}
				delete_node(node, i, index);
				__sync_lock_release(&spin_lock_array[index]);
				total_time_array[(*(int *)thread_counter)] += (stop.tv_sec - start.tv_sec) * 1000000000 + (stop.tv_nsec - start.tv_nsec);
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
void output(struct timespec time_begin, struct timespec time_end) {
	int num_operations = num_iterations * num_threads * 3;
	long long total_time = (time_end.tv_sec - time_begin.tv_sec) * 1000000000 + (time_end.tv_nsec - time_begin.tv_nsec);
	long long avg_time = total_time / ((long long) num_operations);
	long long total_summed_time = 0;
	for(t = 0; t < num_threads; t++) {
		total_summed_time += total_time_array[t];
	}
	long long average_wait_time = total_summed_time / num_operations;
	char str_end[6];
	if(lock == 'n') {
		strcpy(str_end, "-none");
	}
	else {
		strcpy(str_end, "-");
		str_end[1] = lock;
		str_end[2] = '\0';
	}
	fprintf(stdout, "list-%s%s,%d,%d,%d,%d,%lld,%lld,%lld\n", str_yield, str_end, num_threads, num_iterations, num_lists, num_operations, total_time, avg_time, average_wait_time);
}

/**
Main Program Routine:
*/
int main(int argc, char **argv) {
	signal(SIGSEGV, signal_handler);

	// Handle execution argument
	int c = 0;
	static struct option long_options[] = {
		{"threads",		required_argument,	0,	't'},
		{"iterations",	required_argument, 	0,	'i'},
		{"yield",		required_argument,	0,	'y'},
		{"sync",		required_argument,	0,	's'},
		{"lists",		required_argument,	0,	'l'}
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
				for(j = 0; j < strlen(optarg); j++) {
					switch(optarg[j]) {
						case 'i':
							opt_yield |= INSERT_YIELD;
							break;
						case 'd':
							opt_yield |= DELETE_YIELD;
							break;
						case 'l':
							opt_yield |= LOOKUP_YIELD;
							break;
						default:
							fprintf(stderr, "Error, unrecognized yield argument.\n");
							exit(1);
							break;
					}
				}
				strcpy(str_yield, optarg);
				break;
			case 's':
				if(strlen(optarg) == 1) {
					switch(optarg[0]) {
						case 's':
						case 'm':
							lock = optarg[0];
							break;
						default:
							fprintf(stderr, "Error, unrecognized sync argument.\n");
							exit(1);
							break;
					}
				}
				break;
			case 'l':
				num_lists = atoi(optarg);
				break;
			default:
				// TODO: Update Error Message
				fprintf(stderr, "Incorrect Argument Usage, Correct Usage: ./lab1b-client --port=port_number [--log=log_file] [--compress]\n");
				exit(1);
				break;
		}
	}
	
	// Initialize time track array
	total_time_array = malloc(num_threads * sizeof(int));
	for(t = 0; t < num_threads; t++) {
		total_time_array[t] = 0;
	}
	
	// Initialize the list
	list = malloc(num_lists * sizeof(SortedList_t));
	for(t = 0; t < num_lists; t++) {
		list[t].prev = &list[t];
		list[t].next = &list[t];
		list[t].key = NULL;
	}
	
	if(lock == 'm') {
		mutex_array = malloc(num_lists * sizeof(pthread_mutex_t));
		for(t = 0; t < num_lists; t++) {
			if(pthread_mutex_init(&mutex_array[t], NULL)) {
				fprintf(stderr, "Error initializing mutex.\n");
				exit(1);
			}
		}
	}
	
	else if(lock == 's') {
		spin_lock_array = malloc(num_lists * sizeof(int));
		for(t = 0; t < num_lists; t++) {
			spin_lock_array[t] = 0;
		}
	}
	
	element = malloc(sizeof(SortedListElement_t) * num_threads * num_iterations);	
	for(t = 0; t < num_threads * num_iterations; t++) {
		char *random_key = malloc(2 * sizeof(char));
		random_key[0] = 'A' + (rand() % 26);
		random_key[1] = '\0';
		element[t].key = random_key;
	}
	
	// Notes starting time for run
	struct timespec begin;
	if(clock_gettime(CLOCK_MONOTONIC, &begin) == -1) {
		fprintf(stderr, "Error, invalid clock start time.\n");
		exit(1);
	}
	
	// Starts specified number of threads
	threads = malloc(num_threads * sizeof(pthread_t));
	int *thread_counter = malloc(num_threads * sizeof(int));
	
	for(t = 0; t < num_threads; t++) {
		thread_counter[t] = t;
		// Error checking
		if(pthread_create(&threads[t], NULL, thread_exec, &thread_counter[t])) {
			fprintf(stderr, "Error, creating thread number %d failed.\n", t);
			exit(1);
		}
	}
	
	// Joins threads
	for(t = 0; t < num_threads; t++) {
		// Error checking
		if(pthread_join(threads[t], NULL)) {
			fprintf(stderr, "Error, joining thread number %d failed.\n", t);
			exit(1);
		}
	}
		
	// Notes ending time for run
	struct timespec end;
	if(clock_gettime(CLOCK_MONOTONIC, &end) == -1) {
		fprintf(stderr, "Error, invalid clock end time.\n");
		exit(1);
	}
		
	// Cleanup
	free(threads);
	free(thread_counter);
	free(element);
	
	if(lock == 'm') {
		for(t = 0; t < num_lists; t++) {
			pthread_mutex_destroy(&mutex_array[t]);
		}
	}
	else if(lock == 's') {
		free(spin_lock_array);
	}
	
	// Length check
	for(t = 0; t < num_lists; t++) {
		if(SortedList_length(&list[t]) != 0) {
			fprintf(stderr, "Error, list ended with nonzero length.\n");
			exit(2);
		}
	}
	
	// Output
	output(begin, end);
	
	return 0;
}