/*
  NAME: Stewart Dulaney
  EMAIL: sdulaney@ucla.edu
  ID: 904-064-791
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include "SortedList.h"

int num_threads = 0;
int num_iterations = 0;
pthread_mutex_t lock;
int spin_lock = 0;
int opt_yield = 0;
char* arg_sync = NULL;
SortedListElement_t* element_arr = NULL;
char** key_arr = NULL;
SortedList_t head;

void* thread_start_routine(void* elem_arr) {
    SortedListElement_t* arr = elem_arr;
    if (*arg_sync == 'm') {
	if (pthread_mutex_lock(&lock) != 0) {
	    fprintf(stderr, "Error locking mutex.\n");
	    // pthread_mutex_lock isn't a syscall
	    exit(2);
	}
    }
    else if (*arg_sync == 's') {
	while (__sync_lock_test_and_set(&spin_lock, 1)) {
	    continue;
	}
    }
    // Insert all elements into global list
    for (int i = 0; i < num_iterations; i++) {
	SortedList_insert(&head, &arr[i]);
    }
    // Check list length
    if (SortedList_length(&head) < num_iterations) {
	fprintf(stderr, "Error inserting elements.\n");
	exit(2);
    }
    // Look up and delete each of the keys previously inserted
    for (int i = 0; i < num_iterations; i++) {
	SortedListElement_t* element = SortedList_lookup(&head, arr[i].key);
	if (element == NULL) {
	    fprintf(stderr, "Error looking up element.\n");
	    exit(2);
	}
	if (SortedList_delete(element) != 0) {
	    fprintf(stderr, "Error deleting element.\n");
	    exit(2);
	}
    }
    
    if (*arg_sync == 'm') {
	if (pthread_mutex_unlock(&lock) != 0) {
	    fprintf(stderr, "Error unlocking mutex.\n");
	    // pthread_mutex_unlock isn't a syscall
	    exit(2);
	}
    }
    else if (*arg_sync == 's') {
	__sync_lock_release(&spin_lock);
    }
    return NULL;
}

void init_list_elements(int num_elements) {
    element_arr = malloc(sizeof(SortedListElement_t) * num_elements);
    if (element_arr == NULL) {
        fprintf(stderr, "Error allocating memory for elements.\n");
        // malloc isn't a syscall
        exit(2);
    }
    key_arr = malloc(sizeof(char*) * num_elements);
    if (key_arr == NULL) {
        fprintf(stderr, "Error allocating memory for keys.\n");
        // malloc isn't a syscall
        exit(2);
    }
    for (int i = 0; i < num_elements; i++) {
	key_arr[i] = malloc(sizeof(char) * 100);
	if (key_arr[i] == NULL) {
	    fprintf(stderr, "Error allocating memory for keys.\n");
	    // malloc isn't a syscall
	    exit(2);
	}
	for (int j= 0; j < 99; j++) {
	    key_arr[i][j] = rand() % 26 + 'A';
	}
	key_arr[i][100] = '\0';
	element_arr[i].key = key_arr[i];
    }
}

int main(int argc, char** argv) {

    // Process all arguments
    int c;
    int opt_threads = 0;
    int opt_iterations = 0;
    int opt_sync = 0;
    char* arg_threads;
    char* arg_iterations;
    char* arg_yield = NULL;
    char default_val = '1';
    
    while (1) {
        int option_index = 0;
        static struct option long_options[] = {
	    {"threads",    optional_argument, 0,  0 },
	    {"iterations", optional_argument, 0,  0 },
	    {"yield",      optional_argument, 0,  0 },
            {0,         0,                    0,  0 }
        };

        c = getopt_long(argc, argv, "t::i::y:",
	    long_options, &option_index);
        if (c == -1)
	    break;

	const char* name = long_options[option_index].name;
        switch (c) {
	    case 0:
		if (strcmp(name, "threads") == 0) {
		    opt_threads = 1;
                    if (optarg)
			arg_threads = optarg;
		    else
			arg_threads = &default_val;
		}
		else if (strcmp(name, "iterations") == 0) {
		    opt_iterations = 1;
                    if (optarg)
			arg_iterations = optarg;
		    else
			arg_iterations = &default_val;
		}
		else if (strcmp(name, "sync") == 0) {
                    opt_sync = 1;
                    if (optarg)
                        arg_sync = optarg;
                }
		else if (strcmp(name, "yield") == 0) {
                    opt_yield = 1;
		    if (optarg)
                        arg_yield = optarg;
                }
		break;
	    
            case '?':
		fprintf(stderr, "usage: ./lab2_list [OPTION]...\nvalid options: --threads=# (default 1), --iterations=# (default 1), --yield=[idl]\n");
		exit(1);
		break;

            default:
		printf("?? getopt returned character code 0%o ??\n", c);
        }
    }

    // Set default value of 1 for --threads, --iterations if those options aren't given on command line
    if (opt_threads == 0)
	arg_threads = &default_val;
    if (opt_iterations == 0)
        arg_iterations = &default_val;
    num_threads = atoi(arg_threads);
    num_iterations = atoi(arg_iterations);

    long long counter = 0;
    struct timespec start, stop;

    // Initialize an empty list
    head.key = NULL;
    head.next = NULL;
    head.prev = NULL;
    // Create and initialize the required  # of list elements
    init_list_elements(num_threads * num_iterations);
    
    // Note start time
    if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) {
	fprintf(stderr, "Error retrieving time.\nclock_gettime: %s\n", strerror(errno));
        exit(1);
    }

    if (pthread_mutex_init(&lock, NULL) != 0) { 
        fprintf(stderr, "Error initializing mutex.\n"); 
        // pthread_mutex_init isn't a syscall
	exit(2);
    }

    // Start the specified # of threads and wait for all to complete
    pthread_t thread_ids[num_threads];
    for (int i = 0; i < num_threads; i++) {
	int error = pthread_create(&thread_ids[i], NULL, thread_start_routine, (void *) (element_arr + num_iterations * i));
	if (error != 0) {
	    fprintf(stderr, "Error creating thread.\npthread_create: %s\n", strerror(error));
	    // pthread_create isn't a syscall
	    exit(2);
	}
    }
    for (int i = 0; i < num_threads; i++) {
        pthread_join(thread_ids[i], NULL);
    }

    // Note stop time
    if (clock_gettime(CLOCK_MONOTONIC, &stop) == -1) {
        fprintf(stderr, "Error retrieving time.\nclock_gettime: %s\n", strerror(errno));
        exit(1);
    }

    // Check length of list is 0
    if (SortedList_length(&head) != 0) {
	fprintf(stderr, "Error: length of list is not 0 at the end.\n");
        exit(2);
    }

    // Print output data
    long total_run_time = (stop.tv_sec - start.tv_sec) * 1000000000L + (stop.tv_nsec - start.tv_nsec);
    long total_ops = num_threads * num_iterations * 3;
    long avg_time_per_op = total_run_time / total_ops;
    int num_lists = 1;
    // TODO: yieldopts, syncopts

    fprintf(stdout, "list-yieldopts-syncopts,%d,%d,%d,%ld,%ld,%ld\n", num_threads, num_iterations, num_lists, total_ops, total_run_time, avg_time_per_op);

    if (pthread_mutex_destroy(&lock) != 0) {
        fprintf(stderr, "Error destroying mutex.\n");
        // pthread_mutex_destroy isn't a syscall
        exit(2);
    }
    
    exit(0);
}
