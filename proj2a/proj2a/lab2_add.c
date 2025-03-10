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

int opt_yield = 0;
pthread_mutex_t lock;
int spin_lock = 0;

void add(long long * pointer, long long value) {
    long long sum = * pointer + value;
    if (opt_yield)
        sched_yield();
    * pointer = sum;
}

struct thread_data {
    long long * pointer;
    int num_iterations;
};

void * incr_and_decr(void * threadarg) {
    struct thread_data * my_data;
    my_data = (struct thread_data * ) threadarg;
    long long * pointer = my_data -> pointer;
    int n = my_data -> num_iterations;
    for (int i = 0; i < n; i++) {
        add(pointer, 1);
        add(pointer, -1);
    }
    return NULL;
}

void * incr_and_decr_sync_m(void * threadarg) {
    struct thread_data * my_data;
    my_data = (struct thread_data * ) threadarg;
    long long * pointer = my_data -> pointer;
    int n = my_data -> num_iterations;
    for (int i = 0; i < n; i++) {
        // Get lock
        if (pthread_mutex_lock( & lock) != 0) {
            fprintf(stderr, "Error locking mutex.\n");
            exit(1);
        }
        add(pointer, 1);
        // Release lock
        if (pthread_mutex_unlock( & lock) != 0) {
            fprintf(stderr, "Error unlocking mutex.\n");
            exit(1);
        }
        // Get lock
        if (pthread_mutex_lock( & lock) != 0) {
            fprintf(stderr, "Error locking mutex.\n");
            exit(1);
        }
        add(pointer, -1);
        // Release lock
        if (pthread_mutex_unlock( & lock) != 0) {
            fprintf(stderr, "Error unlocking mutex.\n");
            exit(1);
        }
    }
    return NULL;
}

void * incr_and_decr_sync_s(void * threadarg) {
    struct thread_data * my_data;
    my_data = (struct thread_data * ) threadarg;
    long long * pointer = my_data -> pointer;
    int n = my_data -> num_iterations;
    for (int i = 0; i < n; i++) {
        // Get lock
        while (__sync_lock_test_and_set( & spin_lock, 1)) {
            continue;
        }
        add(pointer, 1);
        // Release lock
        __sync_lock_release( & spin_lock);
        // Get lock
        while (__sync_lock_test_and_set( & spin_lock, 1)) {
            continue;
        }
        add(pointer, -1);
        // Release lock
        __sync_lock_release( & spin_lock);
    }
    return NULL;
}

void add_c(long long * pointer, long long value) {
    long long old = 0, new = 0;
    do {
        old = * pointer;
        new = old + value;
        if (opt_yield)
            sched_yield();
    } while (__sync_val_compare_and_swap(pointer, old, new) != old);
}

void * incr_and_decr_sync_c(void * threadarg) {
    struct thread_data * my_data;
    my_data = (struct thread_data * ) threadarg;
    long long * pointer = my_data -> pointer;
    int n = my_data -> num_iterations;
    for (int i = 0; i < n; i++) {
        add_c(pointer, 1);
        add_c(pointer, -1);
    }
    return NULL;
}

int main(int argc, char ** argv) {

    // Process all arguments
    int c;
    int opt_threads = 0;
    int opt_iterations = 0;
    int opt_sync = 0;
    char * arg_threads;
    char * arg_iterations;
    char * arg_sync;
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
                no_argument,
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

        c = getopt_long(argc, argv, "t::i::ys:",
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
            } else if (strcmp(name, "yield") == 0) {
                opt_yield = 1;
            } else if (strcmp(name, "sync") == 0) {
                opt_sync = 1;
                if (optarg)
                    arg_sync = optarg;
            }
            break;

        case '?':
            fprintf(stderr, "usage: ./lab2_add [OPTION]...\nvalid options: --threads=# (default 1), --iterations=# (default 1), --yield, --sync=[msc]\n");
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
    int num_threads = atoi(arg_threads);
    int num_iterations = atoi(arg_iterations);

    long long counter = 0;
    struct timespec start, stop;

    if (pthread_mutex_init( & lock, NULL) != 0) {
        fprintf(stderr, "Error initializing mutex.\n");
        exit(1);
    }

    // Note start time
    if (clock_gettime(CLOCK_MONOTONIC, & start) == -1) {
        fprintf(stderr, "Error retrieving time.\nclock_gettime: %s\n", strerror(errno));
        exit(1);
    }

    pthread_t thread_ids[num_threads];
    struct thread_data threadarg;
    threadarg.pointer = & counter;
    threadarg.num_iterations = num_iterations;
    for (int i = 0; i < num_threads; i++) {
        int error = 0;
        if (opt_sync) {
            if ( * arg_sync == 'm') {
                error = pthread_create( & thread_ids[i], NULL, incr_and_decr_sync_m, (void * ) & threadarg);
            } else if ( * arg_sync == 's') {
                error = pthread_create( & thread_ids[i], NULL, incr_and_decr_sync_s, (void * ) & threadarg);
            } else if ( * arg_sync == 'c') {
                error = pthread_create( & thread_ids[i], NULL, incr_and_decr_sync_c, (void * ) & threadarg);
            }
        } else {
            error = pthread_create( & thread_ids[i], NULL, incr_and_decr, (void * ) & threadarg);
        }
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

    long total_run_time = (stop.tv_sec - start.tv_sec) * 1000000000L + (stop.tv_nsec - start.tv_nsec);
    long total_ops = num_threads * num_iterations * 2;
    long avg_time_per_op = total_run_time / total_ops;
    if (opt_yield) {
        if (!opt_sync) {
            fprintf(stdout, "add-yield-none,%d,%d,%ld,%ld,%ld,%lld\n", num_threads, num_iterations, total_ops, total_run_time, avg_time_per_op, counter);
        } else {
            if ( * arg_sync == 'm') {
                fprintf(stdout, "add-yield-m,%d,%d,%ld,%ld,%ld,%lld\n", num_threads, num_iterations, total_ops, total_run_time, avg_time_per_op, counter);
            } else if ( * arg_sync == 's') {
                fprintf(stdout, "add-yield-s,%d,%d,%ld,%ld,%ld,%lld\n", num_threads, num_iterations, total_ops, total_run_time, avg_time_per_op, counter);
            } else if ( * arg_sync == 'c') {
                fprintf(stdout, "add-yield-c,%d,%d,%ld,%ld,%ld,%lld\n", num_threads, num_iterations, total_ops, total_run_time, avg_time_per_op, counter);
            }
        }
    } else {
        if (opt_sync) {
            if ( * arg_sync == 'm') {
                fprintf(stdout, "add-m,%d,%d,%ld,%ld,%ld,%lld\n", num_threads, num_iterations, total_ops, total_run_time, avg_time_per_op, counter);
            } else if ( * arg_sync == 's') {
                fprintf(stdout, "add-s,%d,%d,%ld,%ld,%ld,%lld\n", num_threads, num_iterations, total_ops, total_run_time, avg_time_per_op, counter);
            } else if ( * arg_sync == 'c') {
                fprintf(stdout, "add-c,%d,%d,%ld,%ld,%ld,%lld\n", num_threads, num_iterations, total_ops, total_run_time, avg_time_per_op, counter);
            }
        } else {
            fprintf(stdout, "add-none,%d,%d,%ld,%ld,%ld,%lld\n", num_threads, num_iterations, total_ops, total_run_time, avg_time_per_op, counter);
        }
    }

    if (pthread_mutex_destroy( & lock) != 0) {
        fprintf(stderr, "Error destroying mutex.\n");
        exit(1);
    }

    exit(0);
}
