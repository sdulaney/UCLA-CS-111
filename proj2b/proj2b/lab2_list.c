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

#include<signal.h>

#include "SortedList.h"

int num_threads = 0;
int num_iterations = 0;
int num_lists = 0;
int opt_yield = 0;
int opt_sync = 0;
char * arg_sync = NULL;
SortedListElement_t * element_arr = NULL;
char ** key_arr = NULL;
SortedList_t* list_arr = NULL;
pthread_mutex_t* lock_arr = NULL;
int* spin_lock_arr = NULL;

struct thread_data {
    SortedListElement_t * elem_arr;
    long mutex_wait_time;
};

long calc_elapsed_time(struct timespec start, struct timespec stop) {
    return (stop.tv_sec - start.tv_sec) * 1000000000L + (stop.tv_nsec - start.tv_nsec);
}

void * thread_start_routine(void * threadarg) {
    struct thread_data * my_data = (struct thread_data * ) threadarg;
    SortedListElement_t * arr = my_data->elem_arr;
    struct timespec start, stop;
    // Use simple modulo operation on first char of key as hash for index into list_arr and lock_arr
    int list_index = 0;
    // Insert all elements into list based on hash of key
    for (int i = 0; i < num_iterations; i++) {
	list_index = arr[i].key[0] % num_lists;
        if (opt_sync && arg_sync != NULL) {
            // Get lock
	    if (clock_gettime(CLOCK_MONOTONIC, & start) == -1) {
		fprintf(stderr, "Error retrieving time.\nclock_gettime: %s\n", strerror(errno));
                exit(1);
            }
            if ( * arg_sync == 'm') {
                if (pthread_mutex_lock( & lock_arr[list_index]) != 0) {
                    fprintf(stderr, "Error locking mutex.\n");
                    exit(1);
                }
            } else if ( * arg_sync == 's') {
                while (__sync_lock_test_and_set( & spin_lock_arr[list_index], 1)) {
                    continue;
                }
            }
	    if (clock_gettime(CLOCK_MONOTONIC, & stop) == -1) {
		fprintf(stderr, "Error retrieving time.\nclock_gettime: %s\n", strerror(errno));
		exit(1);
	    }
	    my_data->mutex_wait_time += calc_elapsed_time(start, stop);
        }
        SortedList_insert( & list_arr[list_index], & arr[i]);
        if (opt_sync && arg_sync != NULL) {
            // Release lock
            if ( * arg_sync == 'm') {
                if (pthread_mutex_unlock( & lock_arr[list_index]) != 0) {
                    fprintf(stderr, "Error unlocking mutex.\n");
                    exit(1);
                }
            } else if ( * arg_sync == 's') {
                __sync_lock_release( & spin_lock_arr[list_index]);
            }
        }
    }
    // Check list length
    if (opt_sync && arg_sync != NULL) {
        // Get lock
	if (clock_gettime(CLOCK_MONOTONIC, & start) == -1) {
	    fprintf(stderr, "Error retrieving time.\nclock_gettime: %s\n", strerror(errno));
            exit(1);
        }
        if ( * arg_sync == 'm') {
	    for (int i = 0; i < num_lists; i++) {
		if (pthread_mutex_lock( & lock_arr[i]) != 0) {
		    fprintf(stderr, "Error locking mutex.\n");
		    exit(1);
		}
	    }
        } else if ( * arg_sync == 's') {
	    for (int i = 0; i < num_lists; i++) {
		while (__sync_lock_test_and_set( & spin_lock_arr[i], 1)) {
		    continue;
		}
	    }
        }
	if (clock_gettime(CLOCK_MONOTONIC, & stop) == -1) {
	    fprintf(stderr, "Error retrieving time.\nclock_gettime: %s\n", strerror(errno));
            exit(1);
        }
        my_data->mutex_wait_time += calc_elapsed_time(start, stop);
    }
    int length = 0;
    for (int i = 0; i < num_lists; i++) {
	length += SortedList_length( & list_arr[i]);
    }
    if (opt_sync && arg_sync != NULL) {
        // Release lock
        if ( * arg_sync == 'm') {
	    for (int i = 0; i < num_lists; i++) {
		if (pthread_mutex_unlock( & lock_arr[i]) != 0) {
		    fprintf(stderr, "Error unlocking mutex.\n");
		    exit(1);
		}
	    }
        } else if ( * arg_sync == 's') {
	    for (int i = 0; i < num_lists; i++) {
		__sync_lock_release( & spin_lock_arr[i]);
	    }
        }
    }
    if (length < num_iterations) {
        fprintf(stderr, "Error inserting elements: got %d instead of %d.\n", length, num_iterations);
        exit(2);
    }

    // Look up and delete each of the keys previously inserted
    for (int i = 0; i < num_iterations; i++) {
	list_index = arr[i].key[0] % num_lists;
        if (opt_sync && arg_sync != NULL) {
            // Get lock
	    if (clock_gettime(CLOCK_MONOTONIC, & start) == -1) {
		fprintf(stderr, "Error retrieving time.\nclock_gettime: %s\n", strerror(errno));
                exit(1);
            }
            if ( * arg_sync == 'm') {
                if (pthread_mutex_lock( & lock_arr[list_index]) != 0) {
                    fprintf(stderr, "Error locking mutex.\n");
                    exit(1);
                }
            } else if ( * arg_sync == 's') {
                while (__sync_lock_test_and_set( & spin_lock_arr[list_index], 1)) {
                    continue;
                }
            }
	    if (clock_gettime(CLOCK_MONOTONIC, & stop) == -1) {
		fprintf(stderr, "Error retrieving time.\nclock_gettime: %s\n", strerror(errno));
		exit(1);
	    }
	    my_data->mutex_wait_time += calc_elapsed_time(start, stop);
        }
        SortedListElement_t * element = SortedList_lookup( & list_arr[list_index], arr[i].key);
        if (element == NULL) {
            fprintf(stderr, "Error looking up element.\n");
            exit(2);
        }
        if (SortedList_delete(element) != 0) {
            fprintf(stderr, "Error deleting element.\n");
            exit(2);
        }
        if (opt_sync && arg_sync != NULL) {
            // Release lock
            if ( * arg_sync == 'm') {
                if (pthread_mutex_unlock( & lock_arr[list_index]) != 0) {
                    fprintf(stderr, "Error unlocking mutex.\n");
                    exit(1);
                }
            } else if ( * arg_sync == 's') {
                __sync_lock_release( & spin_lock_arr[list_index]);
            }
        }
    }
    return NULL;
}

void init(int num_elements) {
    // Initialize list elements and keys to insert into linked list
    element_arr = malloc(sizeof(SortedListElement_t) * num_elements);
    if (element_arr == NULL) {
        fprintf(stderr, "Error allocating memory for elements.\n");
        // malloc isn't a syscall
        exit(2);
    }
    key_arr = malloc(sizeof(char * ) * num_elements);
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
        for (int j = 0; j < 99; j++) {
            key_arr[i][j] = rand() % 26 + 'A';
        }
        key_arr[i][100] = '\0';
        element_arr[i].key = key_arr[i];
    }
    // Initialize array of empty lists
    list_arr = malloc(sizeof(SortedList_t) * num_lists);
    if (list_arr == NULL) {
        fprintf(stderr, "Error allocating memory for array of linked list heads.\n");
        // malloc isn't a syscall
        exit(2);
    }
    for (int i = 0; i < num_lists; i++) {
	list_arr[i].key = NULL;
	list_arr[i].next = NULL;
	list_arr[i].prev = NULL;
    }
    if (opt_sync && arg_sync != NULL) {
	// Initialize array of locks
	if ( * arg_sync == 'm') {
	    lock_arr = malloc(sizeof(pthread_mutex_t) * num_lists);
	    if (lock_arr == NULL) {
		fprintf(stderr, "Error allocating memory for mutex locks.\n");
		// malloc isn't a syscall
		exit(2);
	    }
        } else if ( * arg_sync == 's') {
	    spin_lock_arr = malloc(sizeof(int) * num_lists);
            if (spin_lock_arr == NULL) {
                fprintf(stderr, "Error allocating memory for spin locks.\n");
                // malloc isn't a syscall
                exit(2);
            }
        }
    }
}

void sigsegv_handler() {
    fprintf(stderr, "Error: segmentation fault.\n");
    exit(2);
}

int main(int argc, char ** argv) {

    // Process all arguments
    int c;
    int opt_threads = 0;
    int opt_iterations = 0;
    int opt_lists = 0;
    char * arg_threads = NULL;
    char * arg_iterations = NULL;
    char * arg_yield = NULL;
    char * arg_lists = NULL;
    char default_val = '1';

    while (1) {
        int option_index = 0;
        static struct option long_options[] = {
            {
                "threads",
                optional_argument,
                0,
                0
            },
            {
                "iterations",
                optional_argument,
                0,
                0
            },
            {
                "yield",
                optional_argument,
                0,
                0
            },
            {
                "sync",
                required_argument,
                0,
                0
            },
	    {
                "lists",
                required_argument,
                0,
                0
            },
            {
                0,
                0,
                0,
                0
            }
        };

        c = getopt_long(argc, argv, "t::i::y::s:l:",
            long_options, & option_index);
        if (c == -1)
            break;

        const char * name = long_options[option_index].name;
        switch (c) {
        case 0:
            if (strcmp(name, "threads") == 0) {
                opt_threads = 1;
                if (optarg)
                    arg_threads = optarg;
                else
                    arg_threads = & default_val;
            } else if (strcmp(name, "iterations") == 0) {
                opt_iterations = 1;
                if (optarg)
                    arg_iterations = optarg;
                else
                    arg_iterations = & default_val;
            } else if (strcmp(name, "sync") == 0) {
                opt_sync = 1;
                if (optarg)
                    arg_sync = optarg;
            } else if (strcmp(name, "yield") == 0) {
                if (optarg) {
                    arg_yield = optarg;
                    int len = strlen(arg_yield);
                    for (int i = 0; i < len; i++) {
                        if (optarg[i] == 'i')
                            opt_yield |= INSERT_YIELD;
                        else if (optarg[i] == 'd')
                            opt_yield |= DELETE_YIELD;
                        else if (optarg[i] == 'l')
                            opt_yield |= LOOKUP_YIELD;
                    }
                }
            } else if (strcmp(name, "lists") == 0) {
                opt_lists = 1;
                if (optarg)
                    arg_lists = optarg;
            }
            break;

        case '?':
            fprintf(stderr, "usage: ./lab2_list [OPTION]...\nvalid options: --threads=# (default 1), --iterations=# (default 1), --yield=[idl], --sync=[ms], --lists=# (default 1)\n");
            exit(1);
            break;

        default:
            printf("?? getopt returned character code 0%o ??\n", c);
        }
    }

    // Set default value of 1 for --threads, --iterations, --lists if those options aren't given on command line
    if (opt_threads == 0)
        arg_threads = & default_val;
    if (opt_iterations == 0)
        arg_iterations = & default_val;
    if (opt_lists == 0)
        arg_lists = & default_val;
    num_threads = atoi(arg_threads);
    num_iterations = atoi(arg_iterations);
    num_lists = atoi(arg_lists);

    struct timespec start, stop;

    signal(SIGSEGV, sigsegv_handler);

    // Create and initialize the required # of list elements, locks, and linked list heads
    init(num_threads * num_iterations);

    if (opt_sync && arg_sync != NULL && * arg_sync == 'm') {
	for (int i = 0; i < num_lists; i++) {
	    if (pthread_mutex_init( & lock_arr[i], NULL) != 0) {
		fprintf(stderr, "Error initializing mutex.\n");
		exit(1);
	    }
	}
    }
    
    // Note start time
    if (clock_gettime(CLOCK_MONOTONIC, & start) == -1) {
        fprintf(stderr, "Error retrieving time.\nclock_gettime: %s\n", strerror(errno));
        exit(1);
    }

    // Start the specified # of threads and wait for all to complete
    pthread_t thread_ids[num_threads];
    struct thread_data threadarg_arr[num_threads];
    for (int i = 0; i < num_threads; i++) {
	threadarg_arr[i].elem_arr = element_arr + (num_iterations * i);
	threadarg_arr[i].mutex_wait_time = 0;
        int error = pthread_create( & thread_ids[i], NULL, thread_start_routine, (void * ) &threadarg_arr[i]);
        if (error != 0) {
            fprintf(stderr, "Error creating thread.\npthread_create: %s\n", strerror(error));
            exit(1);
        }
    }
    for (int i = 0; i < num_threads; i++) {
        pthread_join(thread_ids[i], NULL);
    }

    // Note stop time
    if (clock_gettime(CLOCK_MONOTONIC, & stop) == -1) {
        fprintf(stderr, "Error retrieving time.\nclock_gettime: %s\n", strerror(errno));
        exit(1);
    }

    // Check length of each list is 0
    for (int i = 0; i < num_lists; i++) {
	if (SortedList_length( & list_arr[i]) != 0) {
	    fprintf(stderr, "Error: length of list_arr[%d] is not 0 at the end.\n", i);
	    exit(2);
	}
    }

    // Print output data
    long total_run_time = calc_elapsed_time(start, stop);
    long total_ops = num_threads * num_iterations * 3;
    long avg_time_per_op = total_run_time / total_ops;
    long total_mutex_wait_time = 0;
    for (int i = 0; i < num_threads; i++)
	total_mutex_wait_time += threadarg_arr[i].mutex_wait_time;
    int num_lock_ops = num_threads * num_iterations * 3;
    long avg_mutex_wait_per_lock  = total_mutex_wait_time / num_lock_ops;
    char * yieldopts = NULL;
    switch (opt_yield) {
    case 0:
        yieldopts = "none";
        break;
    case 1:
        yieldopts = "i";
        break;
    case 2:
        yieldopts = "d";
        break;
    case 3:
        yieldopts = "id";
        break;
    case 4:
        yieldopts = "l";
        break;
    case 5:
        yieldopts = "il";
        break;
    case 6:
        yieldopts = "dl";
        break;
    case 7:
        yieldopts = "idl";
        break;
    }
    char * syncopts = NULL;
    if (opt_sync == 0) {
        syncopts = "none";
    } else if ( * arg_sync == 'm') {
        syncopts = "m";
    } else if ( * arg_sync == 's') {
        syncopts = "s";
    }

    fprintf(stdout, "list-%s-%s,%d,%d,%d,%ld,%ld,%ld,%ld\n", yieldopts, syncopts, num_threads, num_iterations, num_lists, total_ops, total_run_time, avg_time_per_op, avg_mutex_wait_per_lock);

    if (opt_sync && arg_sync != NULL && * arg_sync == 'm') {
	for (int i = 0; i < num_lists; i++) {
	    if (pthread_mutex_destroy( & lock_arr[i]) != 0) {
		fprintf(stderr, "Error destroying mutex.\n");
		exit(1);
	    }
	}
    }

    exit(0);
}
