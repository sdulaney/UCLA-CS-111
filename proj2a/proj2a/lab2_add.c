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

void add(long long *pointer, long long value) {
    long long sum = *pointer + value;
    *pointer = sum;
}

struct thread_data {
    long long *pointer;
    int num_iterations;
};

void* incr_and_decr(void* threadarg) {
    struct thread_data *my_data;
    my_data = (struct thread_data *) threadarg;
    long long *pointer = my_data->pointer;
    int n = my_data->num_iterations;
    for (int i = 0; i < n; i++) {
	add(pointer, 1);
    }
    for (int i = 0; i < n; i++) {
        add(pointer, -1);
    }
    return NULL;
}

int main(int argc, char** argv) {

    // Process all arguments
    int c;
    int opt_threads = 0;
    int opt_iterations = 0;
    char* arg_threads;
    char* arg_iterations;
    char default_val = '1';
    
    while (1) {
        int option_index = 0;
        static struct option long_options[] = {
	    {"threads",    optional_argument, 0,  0 },
	    {"iterations", optional_argument, 0,  0 },
            {0,         0,                    0,  0 }
        };

        c = getopt_long(argc, argv, "t::i::",
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
		break;
	    
            case '?':
		fprintf(stderr, "usage: ./lab2_add [OPTION]...\nvalid options: --threads=# (default 1), --iterations=# (default 1)\n");
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
    int num_threads = atoi(arg_threads);
    int num_iterations = atoi(arg_iterations);

    long long counter = 0;
    struct timespec start, stop;
    
    // Note start time
    if (clock_gettime(CLOCK_REALTIME, &start) == -1) {
	fprintf(stderr, "Error retrieving time.\nclock_gettime: %s\n", strerror(errno));
        exit(1);
    }

    pthread_t thread_ids[num_threads];
    struct thread_data threadarg;
    threadarg.pointer = &counter;
    threadarg.num_iterations = num_iterations;
    for (int i = 0; i < num_threads; i++) {
	int error = pthread_create(&thread_ids[i], NULL, incr_and_decr, (void *) &threadarg);
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
    if (clock_gettime(CLOCK_REALTIME, &stop) == -1) {
        fprintf(stderr, "Error retrieving time.\nclock_gettime: %s\n", strerror(errno));
        exit(1);
    }

    long total_run_time = (stop.tv_sec - start.tv_sec) * 1000000000L + (stop.tv_nsec - start.tv_nsec);
    long total_ops = num_threads * num_iterations * 2;
    long avg_time_per_op = total_run_time / total_ops;
    fprintf(stdout, "add-none,%d,%d,%ld,%ld,%ld,%lld\n", num_threads, num_iterations, total_ops, total_run_time, avg_time_per_op, counter);

    exit(0);
}
