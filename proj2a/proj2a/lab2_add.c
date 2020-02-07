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

void add(long long *pointer, long long value) {
    long long sum = *pointer + value;
    *pointer = sum;
}

int main(int argc, char** argv) {

    // Process all arguments
    int c;
    int opt_threads = 0;
    int opt_iterations = 0;
    char* arg_threads;
    char* arg_iterations;
    char default = '1';
    
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
			arg_threads = &default;
		}
		else if (strcmp(name, "iterations") == 0) {
		    opt_iterations = 1;
                    if (optarg)
			arg_iterations = optarg;
		    else
			arg_iterations = &default;
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

    exit(0);
}
