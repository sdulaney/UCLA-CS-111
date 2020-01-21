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
    int opt_shell = 0;
    
    while (1) {
        int option_index = 0;
        static struct option long_options[] = {
	    {"shell",         no_argument, 0,  0 },
            {0,         0,                 0,  0 }
        };

        c = getopt_long(argc, argv, "s",
	    long_options, &option_index);
        if (c == -1)
	    break;

	const char* name = long_options[option_index].name;
        switch (c) {
	    case 0:
		if (strcmp(name, "shell") == 0) {
		    opt_shell = 1;
		}
		break;
		
            case 's':
		printf("option s\n");
		break;

            case '?':
		fprintf(stderr, "usage: ./lab1a [OPTION]\nvalid option(s): --shell\n");
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

    if (opt_shell == 1) {
	pid_t rv = fork();
	if (rv == -1) {
	    fprintf(stderr, "Error creating child process.\nfork: %s\n", strerror(errno));
	    exit(1);
	}
	else if (rv == 0) {
	    // Returned to child process
	    char* args[] = {};
	    int rv = execv("/bin/bash", args);
	    if (rv == -1) {
		fprintf(stderr, "Error exec'ing a shell.\nexecv: %s\n", strerror(errno));
		exit(1);
	    }

	    
	    
	}
	else if (rv > 0) {
	    // Returned to parent process
	    
	}
    }
    else {
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
    }

    exit(0);
}
