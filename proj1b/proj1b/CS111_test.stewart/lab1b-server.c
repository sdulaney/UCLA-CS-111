/*
  NAME: Stewart Dulaney
  EMAIL: sdulaney@ucla.edu
  ID: 904-064-791

  File: lab1b-server.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <termios.h>
#include <sys/poll.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

struct termios termios_curr;
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
    fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d", status & 0x007f, (status & 0xff00) >> 8);
    exit(0);
}

void sigpipe_handler(int signum) {
    if (signum == SIGPIPE) {
	print_shell_exit_info(pid);
    }
}

void reset_input_mode() {
    // Restore the terminal modes
    tcsetattr(0, TCSANOW, &termios_curr);
}


int main(int argc, char** argv) {

    // Process all arguments
    int c;
    int opt_port = 0;
    char* arg_port;
    int opt_compress = 0;
    
    while (1) {
        int option_index = 0;
        static struct option long_options[] = {
	    {"port",    required_argument, 0,  0 },
	    {"compress",      no_argument, 0,  0 },
            {0,         0,                 0,  0 }
        };

        c = getopt_long(argc, argv, "p:c",
	    long_options, &option_index);
        if (c == -1)
	    break;

	const char* name = long_options[option_index].name;
        switch (c) {
	    case 0:
		if (strcmp(name, "port") == 0) {
		    opt_port = 1;
		    if (optarg)
			arg_port = optarg;
		}
		if (strcmp(name, "compress") == 0) {
                    opt_compress = 1;
                }
		break;
		
            case 'p':
		printf("option p\n");
		break;

            case '?':
		fprintf(stderr, "usage: ./lab1b-server [OPTION]...\nvalid option(s): ----port=port, --compress\n");
		exit(1);
		break;

            default:
		printf("?? getopt returned character code 0%o ??\n", c);
        }
    }

    // Get and save the current terminal modes
    tcgetattr(0, &termios_curr);

    // Put the keyboard into character-at-a-time, no-echo mode
    struct termios termios_new = termios_curr;
    termios_new.c_iflag = ISTRIP;             /* only lower 7 bits	*/
    termios_new.c_oflag = 0;                  /* no processing	*/
    termios_new.c_lflag = 0;                  /* no processing	*/
    tcsetattr(0, TCSANOW, &termios_new);

    atexit(reset_input_mode);

    if (opt_port == 1) {
	// Socket set up
	int sockfd, newsockfd, portno;
	socklen_t clilen;
//	char buffer[256];
	struct sockaddr_in serv_addr, cli_addr;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
	    fprintf(stderr, "Error opening socket.\n");
            exit(1);
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = atoi(arg_port);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
	    fprintf(stderr, "Error on binding.\n");
            exit(1);
	}
	listen(sockfd, 5);
	clilen = sizeof(cli_addr);
	newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	if (newsockfd < 0) {
	    fprintf(stderr, "Error on accept.\n");
            exit(1);
	}

        if (opt_compress == 1) {

        }
	
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
	    // Create an array of two pollfd structures, describing the input received from the TCP connection with client and the shell output received from the pipe
	    struct pollfd fds[2];
	    fds[0].fd = newsockfd;
	    fds[0].events = POLLIN;
	    fds[0].revents = 0;
	    fds[1].fd = pipe_to_term[0];
	    fds[1].events = POLLIN;
	    fds[1].revents = 0;
	    while (!closed_write_pipe_to_shell) {
		int rv_poll = poll(fds, 2, 0);
		if (rv_poll == -1) {
		    fprintf(stderr, "Error polling for events on file descriptors %d and %d.\npoll: %s\n", newsockfd, pipe_to_term[0], strerror(errno));
		    exit(1);
		}
		if (rv_poll > 0) {
		    if (fds[0].revents & POLLIN) {
			// Read from client
			// Read (ASCII) input from the client, and forward it to the shell
			char input[100];
			    ssize_t rv = read(fds[0].fd, &input, 99);
			    if (rv == -1) {
				fprintf(stderr, "Error reading from file descriptor %d.\nread: %s\n", fds[0].fd, strerror(errno));
				exit(1);
			    }
			    for (int i = 0; i < rv; i++) {
				if (input[i] == '\x04') {
				    // Close the pipe to the shell
                                    close(pipe_to_shell[1]);
				    closed_write_pipe_to_shell = 1;
				}
				else if (input[i] == '\x03') {
				    // Send a SIGINT to the shell process
				    if (kill(pid, SIGINT) == -1) {
					fprintf(stderr, "Error sending signal to process with PID %d.\nkill: %s\n", pid, strerror(errno));
					exit(1);
				    }
				}
				else {
				    // Forward to shell
				    ssize_t temp = write(pipe_to_shell[1], &input[i], 1);
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
				// Close the pipe to the shell
				close(pipe_to_shell[1]);
				closed_write_pipe_to_shell = 1;
			    }
			    if (rv_shell_out == -1) {
				fprintf(stderr, "Error reading from file descriptor %d.\nread: %s\n", pipe_to_term[0], strerror(errno));
				exit(1);
			    }
			    for (int i = 0; i < rv_shell_out; i++) {
				    ssize_t temp = write(newsockfd, &shell_out[i], 1);
				    if (temp < 1) {              /* # bytes written may be less than arg count */
                                        fprintf(stderr, "Error writing to file descriptor %d.\nwrite: %s\n", newsockfd, strerror(errno));
                                        exit(1);
                                    }
			    }
		    }
		}
	    }
	    print_shell_exit_info(pid);
    }
    else {
	fprintf(stderr, "Error: the option --port=port is mandatory.\nusage: ./lab1b-server [OPTION]...\nvalid option(s): ----port=port, --compress\n");
        exit(1);
    }
    exit(0);
}
