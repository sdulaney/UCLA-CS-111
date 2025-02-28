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
#include <sys/poll.h>
#include <sys/wait.h>

int pipe_to_shell[2];
int closed_write_pipe_to_shell = 0;
int eof_from_shell = 0;
pid_t pid = 0;

void print_shell_exit_info(pid_t pid) {
    int status = 0;
    if (waitpid(pid, &status, 0) == -1) {
	fprintf(stderr, "Error waiting for process to change state.\nwaitpid: %s\n", strerror(errno));
	exit(1);
    }
    fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d", status & 0x007f, status & 0xff00);
    exit(0);
}

void sigpipe_handler(int signum) {
    if (signum == SIGPIPE) {
	// Close the pipe to the shell
//	close(pipe_to_shell[1]);
//	closed_write_pipe_to_shell = 1;
	print_shell_exit_info(pid);
    }
}


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
	int rv_pipe = pipe(pipe_to_shell);
	if (rv_pipe == -1) {
	    fprintf(stderr, "Error creating pipe.\npipe: %s\n", strerror(errno));
	    exit(1);
	}
	int pipe_to_term[2];
	rv_pipe = pipe(pipe_to_term);
	if (rv_pipe == -1) {
	    fprintf(stderr, "Error creating pipe.\npipe: %s\n", strerror(errno));
	    exit(1);
	}
	
	pid = fork();
	if (pid == -1) {
	    fprintf(stderr, "Error creating child process.\nfork: %s\n", strerror(errno));
	    exit(1);
	}
	else if (pid == 0) {
	    // Returned to child process
	    // Close unused side of pipes
	    close(pipe_to_shell[1]);
	    close(pipe_to_term[0]);
	    // Redirect stdin
	    close(0);
	    dup(pipe_to_shell[0]);
	    close(pipe_to_shell[0]);
	    // Redirect stdout
	    close(1);
	    dup(pipe_to_term[1]);
	    close(pipe_to_term[1]);
	    // Redirect stderr
	    close(2);
	    dup(1);
	    char* args[] = {"/bin/bash", NULL};
	    int rv = execv("/bin/bash", args);
	    if (rv == -1) {
		fprintf(stderr, "Error exec'ing a shell.\nexecv: %s\n", strerror(errno));
		exit(1);
	    }
	}
	else if (pid > 0) {
	    // Returned to parent process
	    signal(SIGPIPE, sigpipe_handler);
	    // Close unused side of pipes
	    close(pipe_to_shell[0]);
	    close(pipe_to_term[1]);
	    // Create an array of two pollfd structures, one describing the keyboard (stdin) and one describing the pipe that returns output from the shell
	    struct pollfd fds[2];
	    fds[0].fd = 0;
	    fds[0].events = POLLIN;
	    fds[0].revents = 0;
	    fds[1].fd = pipe_to_term[0];
	    fds[1].events = POLLIN;
	    fds[1].revents = 0;
	    while (1) {
		int rv_poll = poll(fds, 2, 0);
		if (rv_poll == -1) {
		    fprintf(stderr, "Error polling for events on file descriptors 0 and %d.\npoll: %s\n", pipe_to_term[0], strerror(errno));
		    exit(1);
		}
		if (!closed_write_pipe_to_shell && rv_poll > 0) {
		    // TODO: deal with POLLERR, POLLHUP
		    /*if (fds[0].revents & POLLERR || fds[1].revents & POLLERR) {
			
		      }*/
		    if (fds[0].revents & POLLIN) {
			// Read stdin
			// Read (ASCII) input from the keyboard, echo it to stdout, and forward it to the shell
			char input[100];
			    ssize_t rv = read(0, &input, 99);
			    if (rv == -1) {
				fprintf(stderr, "Error reading from file descriptor 0.\nread: %s\n", strerror(errno));
				exit(1);
			    }
			    for (int i = 0; i < rv; i++) {
				if (input[i] == '\x0D' || input[i] == '\x0A') {
				    // Echo to stdout
				    char* output = "\x0D\x0A";
				    ssize_t temp = write(0, output, 2);
				    if (temp < 2) {               /* # bytes written may be less than arg count */
					fprintf(stderr, "Error writing to file descriptor 0.\nwrite: %s\n", strerror(errno));
					exit(1);
				    }
				    // Forward to shell
				    char c = '\x0A';
				    temp = write(pipe_to_shell[1], &c, 1);
				    if (temp < 1) {              /* # bytes written may be less than arg count */
					fprintf(stderr, "Error writing to file descriptor %d.\nwrite: %s\n", pipe_to_shell[1], strerror(errno));
					exit(1);
				    }
				}
				else if (input[i] == '\x04') {
				    // Echo to stdout
				    char* str = "^D";
                                    if (write(0, str, 2) < 2) {
                                        fprintf(stderr, "Error writing to file descriptor 0.\nwrite: %s\n", strerror(errno));
                                        exit(1);
                                    }
				    // Close the pipe to the shell
                                    close(pipe_to_shell[1]);
				    closed_write_pipe_to_shell = 1;
				    // Restore the terminal modes
				    tcsetattr(0, TCSANOW, &termios_curr);
				    exit(0);
				}
				else if (input[i] == '\x03') {
				    // Echo to stdout
				    char* str = "^C";
				    if (write(0, str, 2) < 2) {
					fprintf(stderr, "Error writing to file descriptor 0.\nwrite: %s\n", strerror(errno));
					exit(1);
				    }
				    // Send a SIGINT to the shell process
				    if (kill(pid, SIGINT) == -1) {
					fprintf(stderr, "Error sending signal to process with PID %d.\nkill: %s\n", pid, strerror(errno));
					exit(1);
				    }
				}
				else {
				    // Echo to stdout
				    ssize_t temp = write(0, &input[i], 1);
				    if (temp < 1) {              /* # bytes written may be less than arg count */
					fprintf(stderr, "Error writing to file descriptor 0.\nwrite: %s\n", strerror(errno));
					exit(1);
				    }
				    // Forward to shell
				    temp = write(pipe_to_shell[1], &input[i], 1);
				    if (temp < 1) {              /* # bytes written may be less than arg count */
					fprintf(stderr, "Error writing to file descriptor %d.\nwrite: %s\n", pipe_to_shell[1], strerror(errno));
					exit(1);
				    }
				}
			    }
			}
		    }
		    if (fds[1].revents & POLLERR || fds[1].revents & POLLHUP) {
			// Close the pipe to the shell
			close(pipe_to_shell[1]);
			closed_write_pipe_to_shell = 1;
		    }
		    if (fds[1].revents & POLLIN) {
			// Read output from shell
			char shell_out[512];
			    ssize_t rv_shell_out = read(pipe_to_term[0], &shell_out, 511);
			    if (rv_shell_out == 0) {     // EOF
				eof_from_shell = 1;
			    }
			    if (rv_shell_out == -1) {
				fprintf(stderr, "Error reading from file descriptor %d.\nread: %s\n", pipe_to_term[0], strerror(errno));
				exit(1);
			    }
			    for (int i = 0; i < rv_shell_out; i++) {
				if (shell_out[i] == '\x0A') {
				    char* output = "\x0D\x0A";
				    ssize_t temp = write(0, output, 2);
                                    if (temp < 2) {               /* # bytes written may be less than arg count */
                                        fprintf(stderr, "Error writing to file descriptor 0.\nwrite: %s\n", strerror(errno));
                                        exit(1);
                                    }
				}
				else {
				    ssize_t temp = write(0, &shell_out[i], 1);
				    if (temp < 1) {              /* # bytes written may be less than arg count */
                                        fprintf(stderr, "Error writing to file descriptor 0.\nwrite: %s\n", strerror(errno));
                                        exit(1);
                                    }
				}
			    }
		    }
		    if (eof_from_shell && closed_write_pipe_to_shell) {
			print_shell_exit_info(pid);
		    }
		}
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
