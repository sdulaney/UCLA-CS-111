/*
  NAME: Stewart Dulaney
  EMAIL: sdulaney@ucla.edu
  ID: 904-064-791
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>

void sigsev_handler(int signum) {
    fprintf(stderr, "Error: segmentation fault.\n");
    exit(4);
}

void seg_fault() {
    char* ptr = NULL;
    *ptr = 'f';
}

int main(int argc, char** argv) {

    // Process all arguments
    int c;
    int opt_input = 0;
    int opt_output = 0;
    int opt_segfault = 0;
    int opt_catch = 0;
    char* arg_input;
    char* arg_output;
    
    while (1) {
        int option_index = 0;
        static struct option long_options[] = {
	    {"input",   required_argument, 0,  0 },
	    {"output",  required_argument, 0,  0 },
	    {"segfault",      no_argument, 0,  0 },
	    {"catch",         no_argument, 0,  0 },
            {0,         0,                 0,  0 }
        };

        c = getopt_long(argc, argv, "i:o:sc",
	    long_options, &option_index);
        if (c == -1)
	    break;

	const char* name = long_options[option_index].name;
        switch (c) {
	    case 0:
		if (strcmp(name, "input") == 0) {
		    opt_input = 1;
                    if (optarg)
			arg_input = optarg;
		}
		else if (strcmp(name, "output") == 0) {
		    opt_output = 1;
                    if (optarg)
			arg_output = optarg;
		}
		else if (strcmp(name, "segfault") == 0) {
		    opt_segfault = 1;
		}
		else if (strcmp(name, "catch") == 0) {
		    opt_catch = 1;
		}
		break;
	    
            case 'i':
		printf("option i with value '%s'\n", optarg);
		break;

            case 'o':
		printf("option o with value '%s'\n", optarg);
		break;

            case 's':
		printf("option s\n");
		break;

            case 'c':
		printf("option c\n");
		break;

            case '?':
		break;

            default:
		printf("?? getopt returned character code 0%o ??\n", c);
        }
    }

    // Do any file redirection
    if (opt_input) {
	int ifd = open(arg_input, O_RDONLY);
	if (ifd >= 0) {
	    close(0);
	    dup(ifd);
	    close(ifd);
	}
	printf("option input with arg %s\n", arg_input);
    }
    if (opt_output) {
	int ofd = creat(arg_output, 0666);
	if (ofd >= 0) {
	    close(1);
	    dup(ofd);
	    close(ofd);
	}
        printf("option output with arg %s\n", arg_output);
    }

    // Register the signal handler
    if (opt_catch) {
	signal(SIGSEGV, sigsev_handler);
	printf("option catch\n");
    }

    // Cause the segfault
    if (opt_segfault) {
	seg_fault();
        printf("option segfault\n");
    }

    // Copy STDIN to STDOUT 
    int buf;
    while (1) {
	int temp = read(0, &buf, 1);

	// EOF
	if (temp == 0) {
	    break;
	}

	write(1, &buf, 1);
    }
    exit(0);
}
