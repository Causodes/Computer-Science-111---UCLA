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
*/

/**
Global Variables:
*/
int num_threads = 1;
int num_iterations = 1;
pthread_t *threads;
int t;
int opt_yield;
unsigned int j;
char lock = 'n';
int spin_lock = 0;
pthread_mutex_t mutex;
SortedList_t * list;
SortedListElement_t * element;
int list_length = 0;
char str_yield[5] = "none";


void signal_handler() {
  fprintf(stderr, "SIGNAL SEGFAULT\n");
  exit(1);
}

/**
Node Delete Function:
*/
void delete_node(SortedListElement_t *node, int i) {
	if((node = SortedList_lookup(list, element[i].key))) {
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
	int i;
	for(i = (*(int *)thread_counter); i < num_threads * num_iterations; i+=num_threads) {
		switch(lock) {
			case 'n':
				SortedList_insert(list, &element[i]);
				break;
			case 'm':
				pthread_mutex_lock(&mutex);
				SortedList_insert(list, &element[i]);
				pthread_mutex_unlock(&mutex);
				break;
			case 's':
				while (__sync_lock_test_and_set(&spin_lock, 1));
				SortedList_insert(list, &element[i]);
				__sync_lock_release(&spin_lock);
				break;
			default:
				fprintf(stderr, "Error, unrecognized sync argument.\n");
				exit(1);
				break;
		}
	}
	list_length = SortedList_length(list);
	
	SortedListElement_t *node = NULL;
	for(i = (*(int *)thread_counter); i < num_threads * num_iterations; i+=num_threads) {
		switch(lock) {
			case 'n':
				delete_node(node, i);
				break;
			case 'm':
				pthread_mutex_lock(&mutex);
				delete_node(node, i);
				pthread_mutex_unlock(&mutex);
				break;
			case 's':
				while (__sync_lock_test_and_set(&spin_lock, 1));
				delete_node(node, i);
				__sync_lock_release(&spin_lock);
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
void output(int thread_num, int iteration_num, struct timespec time_begin, struct timespec time_end) {
	int num_operations = thread_num * iteration_num * 3;
	long long total_time = (time_end.tv_sec - time_begin.tv_sec) * 1000000000 + (time_end.tv_nsec - time_begin.tv_nsec);
	long long avg_time = total_time/ ((long long) num_operations);
	char str_end[6];
	if(lock == 'n') {
		strcpy(str_end, "-none");
	}
	else {
		strcpy(str_end, "-");
		str_end[1] = lock;
		str_end[2] = '\0';
	}
	fprintf(stdout, "list-%s%s,%d,%d,1,%d,%lld,%lld\n", str_yield, str_end, thread_num, iteration_num, num_operations, total_time, avg_time);
}

/**
Main Program Routine:
*/
int main(int argc, char **argv) {
	signal(SIGSEGV, signal_handler);

	// Handle execution argument
	int c = 0;
	// TODO: Add yield and sync arguments
	static struct option long_options[] = {
		{"threads",		required_argument,	0,	't'},
		{"iterations",	required_argument, 	0,	'i'},
		{"yield",		required_argument,	0,	'y'},
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
						case 'c':
							lock = optarg[0];
							break;
						case 'm':
							lock = optarg[0];
							if(pthread_mutex_init(&mutex, NULL)) {
								fprintf(stderr, "Error initializing mutex.\n");
								exit(1);
							}
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

	// Initialize the list
	list = malloc(sizeof(SortedList_t));
	list -> prev = list;
	list -> next = list;
	list -> key = NULL;
	
	element = malloc(sizeof(SortedListElement_t) * num_threads * num_iterations);
	int i;
	for(i = 0; i < num_threads * num_iterations; i++) {
		char *random_key = malloc(2 * sizeof(char));
		random_key[0] = 'A' + (rand() % 26);
		random_key[1] = '\0';
		element[i].key = random_key;
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
		//printf("Creating thread...\n");
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
			free(threads);
			exit(1);
		}
	}
		
	// Cleanup
	free(threads);
	free(thread_counter);
	if(lock == 'm') {
		pthread_mutex_destroy(&mutex);
	}
	
	// Notes ending time for run
	struct timespec end;
	if(clock_gettime(CLOCK_MONOTONIC, &end) == -1) {
		fprintf(stderr, "Error, invalid clock end time.\n");
		exit(1);
	}
	
	// Length check
	if(SortedList_length(list) != 0) {
		fprintf(stderr, "Error, final list length not 0.\n");
		exit(2);
	}
	output(num_threads, num_iterations, begin, end);
	
	return 0;
}