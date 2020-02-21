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
pthread_mutex_t lock;
int spin_lock = 0;
int opt_yield = 0;
int opt_sync = 0;
char * arg_sync = NULL;
SortedListElement_t * element_arr = NULL;
char ** key_arr = NULL;
SortedList_t head;

void * thread_start_routine(void * elem_arr) {
  SortedListElement_t * arr = elem_arr;
  // Insert all elements into global list
  for (int i = 0; i < num_iterations; i++) {
    if (opt_sync && arg_sync != NULL) {
      if ( * arg_sync == 'm') {
        if (pthread_mutex_lock( & lock) != 0) {
          fprintf(stderr, "Error locking mutex.\n");
          exit(1);
        }
      } else if ( * arg_sync == 's') {
        while (__sync_lock_test_and_set( & spin_lock, 1)) {
          continue;
        }
      }
    }
    SortedList_insert( & head, & arr[i]);
    if (opt_sync && arg_sync != NULL) {
      if ( * arg_sync == 'm') {
        if (pthread_mutex_unlock( & lock) != 0) {
          fprintf(stderr, "Error unlocking mutex.\n");
          exit(1);
        }
      } else if ( * arg_sync == 's') {
        __sync_lock_release( & spin_lock);
      }
    }
  }
  // Check list length
  if (opt_sync && arg_sync != NULL) {
    if ( * arg_sync == 'm') {
      if (pthread_mutex_lock( & lock) != 0) {
        fprintf(stderr, "Error locking mutex.\n");
        exit(1);
      }
    } else if ( * arg_sync == 's') {
      while (__sync_lock_test_and_set( & spin_lock, 1)) {
        continue;
      }
    }
  }
  int length = SortedList_length( & head);
  if (opt_sync && arg_sync != NULL) {
    if ( * arg_sync == 'm') {
      if (pthread_mutex_unlock( & lock) != 0) {
        fprintf(stderr, "Error unlocking mutex.\n");
        exit(1);
      }
    } else if ( * arg_sync == 's') {
      __sync_lock_release( & spin_lock);
    }
  }
  if (length < num_iterations) {
    fprintf(stderr, "Error inserting elements: got %d instead of %d.\n", length, num_iterations);
    exit(2);
  }

  // Look up and delete each of the keys previously inserted
  for (int i = 0; i < num_iterations; i++) {
    if (opt_sync && arg_sync != NULL) {
      if ( * arg_sync == 'm') {
        if (pthread_mutex_lock( & lock) != 0) {
          fprintf(stderr, "Error locking mutex.\n");
          exit(1);
        }
      } else if ( * arg_sync == 's') {
        while (__sync_lock_test_and_set( & spin_lock, 1)) {
          continue;
        }
      }
    }
    SortedListElement_t * element = SortedList_lookup( & head, arr[i].key);
    if (element == NULL) {
      fprintf(stderr, "Error looking up element.\n");
      exit(2);
    }
    if (SortedList_delete(element) != 0) {
      fprintf(stderr, "Error deleting element.\n");
      exit(2);
    }
    if (opt_sync && arg_sync != NULL) {
      if ( * arg_sync == 'm') {
        if (pthread_mutex_unlock( & lock) != 0) {
          fprintf(stderr, "Error unlocking mutex.\n");
          exit(1);
        }
      } else if ( * arg_sync == 's') {
        __sync_lock_release( & spin_lock);
      }
    }
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
  char * arg_threads;
  char * arg_iterations;
  char * arg_yield = NULL;
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
        0,
        0,
        0,
        0
      }
    };

    c = getopt_long(argc, argv, "t::i::y::s:",
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
      }
      break;

    case '?':
      fprintf(stderr, "usage: ./lab2_list [OPTION]...\nvalid options: --threads=# (default 1), --iterations=# (default 1), --yield=[idl], --sync=[ms]\n");
      exit(1);
      break;

    default:
      printf("?? getopt returned character code 0%o ??\n", c);
    }
  }

  // Set default value of 1 for --threads, --iterations if those options aren't given on command line
  if (opt_threads == 0)
    arg_threads = & default_val;
  if (opt_iterations == 0)
    arg_iterations = & default_val;
  num_threads = atoi(arg_threads);
  num_iterations = atoi(arg_iterations);

  struct timespec start, stop;

  signal(SIGSEGV, sigsegv_handler);

  // Initialize an empty list
  head.key = NULL;
  head.next = NULL;
  head.prev = NULL;
  // Create and initialize the required  # of list elements
  init_list_elements(num_threads * num_iterations);

  // Note start time
  if (clock_gettime(CLOCK_MONOTONIC, & start) == -1) {
    fprintf(stderr, "Error retrieving time.\nclock_gettime: %s\n", strerror(errno));
    exit(1);
  }

  if (pthread_mutex_init( & lock, NULL) != 0) {
    fprintf(stderr, "Error initializing mutex.\n");
    exit(1);
  }

  // Start the specified # of threads and wait for all to complete
  pthread_t thread_ids[num_threads];
  for (int i = 0; i < num_threads; i++) {
    int error = pthread_create( & thread_ids[i], NULL, thread_start_routine, (void * )(element_arr + num_iterations * i));
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

  // Check length of list is 0
  if (SortedList_length( & head) != 0) {
    fprintf(stderr, "Error: length of list is not 0 at the end.\n");
    exit(2);
  }

  // Print output data
  long total_run_time = (stop.tv_sec - start.tv_sec) * 1000000000 L + (stop.tv_nsec - start.tv_nsec);
  long total_ops = num_threads * num_iterations * 3;
  long avg_time_per_op = total_run_time / total_ops;
  int num_lists = 1;
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

  fprintf(stdout, "list-%s-%s,%d,%d,%d,%ld,%ld,%ld\n", yieldopts, syncopts, num_threads, num_iterations, num_lists, total_ops, total_run_time, avg_time_per_op);

  if (pthread_mutex_destroy( & lock) != 0) {
    fprintf(stderr, "Error destroying mutex.\n");
    exit(1);
  }

  exit(0);
}
