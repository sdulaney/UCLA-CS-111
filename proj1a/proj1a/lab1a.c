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
#include <errno.h>
#include <termios.h>

int main(int argc, char** argv) {

    // Process all arguments
    int c;
/*    int opt_input = 0;
    int opt_output = 0;
    int opt_segfault = 0;
    int opt_catch = 0;
    char* arg_input;
    char* arg_output;*/
    
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

	//const char* name = long_options[option_index].name;
        switch (c) {
	    case 0:
/*		if (strcmp(name, "input") == 0) {
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
		    }*/
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
		fprintf(stderr, "usage: ./lab0 [OPTION]...\nvalid options: --input=filename, --output=filename, --segfault, --catch\n");
		exit(1);
		break;

            default:
		printf("?? getopt returned character code 0%o ??\n", c);
        }
    }

    // Get and save the current terminal modes
    struct termios termios_curr;
    tcgetattr(0, &termios_curr);

    // Put the keyboard into character-at-a-time, no-echo mode
    struct termios termios_new = termios_curr;
    termios_new.c_iflag = ISTRIP;             /* only lower 7 bits	*/
    termios_new.c_oflag = 0;                  /* no processing	*/
    termios_new.c_lflag = 0;                  /* no processing	*/
    tcsetattr(0, TCSANOW, &termios_new);

    // Read (ASCII) input from the keyboard into a buffer
    char input[100];
    while (1) {
	ssize_t rv = read(0, &input, 99);
	if (rv == -1) {
	    fprintf(stderr, "Error reading from file descriptor 0.\nread: %s\n", strerror(errno));
	    exit(1);
	}
	for (int i = 0; i < rv; i++) {
	    if (input[i] == '\x0D' || input[i] == '\x0A') {
		char* output = "\x0D\x0A";
		ssize_t temp = write(0, output, 2);
		if (temp < 2) {               /* # bytes written may be less than arg count */
		    fprintf(stderr, "Error writing to file descriptor 0.\nwrite: %s\n", strerror(errno));
		    exit(1);
		}		
	    }
	    else if (input[i] == '\x04') {
		// Restore the terminal modes
		tcsetattr(0, TCSANOW, &termios_curr);
		exit(0);
	    }
	    else {
		ssize_t temp = write(0, &input[i], 1);
		if (temp < 1) {              /* # bytes written may be less than arg count */
		    fprintf(stderr, "Error writing to file descriptor 0.\nwrite: %s\n", strerror(errno));
		    exit(1);
		}
	    }
	}
    }

    exit(0);
}
