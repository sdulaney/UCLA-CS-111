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
                required_argument,
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

        c = getopt_long(argc, argv, "t::i::y:s:l:",
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

    exit(0);
}
