/*
  NAME: Stewart Dulaney
  EMAIL: sdulaney@ucla.edu
  ID: 904-064-791

  File: lab1b-client.c
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
#include <netdb.h>
#include <fcntl.h>
#include "zlib.h"

struct termios termios_curr;
int pipe_to_shell[2];
int server_exited = 0;
int eof_from_shell = 0;
pid_t pid = 0;
int logfd;

void print_shell_exit_info(pid_t pid) {
    int status = 0;
    if (waitpid(pid, &status, 0) == -1) {
	fprintf(stderr, "Error waiting for process to change state.\nwaitpid: %s\n", strerror(errno));
	exit(1);
    }
    fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d", status & 0x007f, (status & 0xff00) >> 8);
    exit(0);
}

void print_log(int sent, int fd, char* buf, int count) {
    if (sent) {
	dprintf(fd, "SENT %d bytes: %s", count, buf);
    }
    else {
	dprintf(fd, "RECEIVED %d bytes: %s", count, buf);
    }
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
    int opt_log = 0;
    char* arg_log;
    int opt_compress = 0;
    
    while (1) {
        int option_index = 0;
        static struct option long_options[] = {
	    {"port",    required_argument, 0,  0 },
	    {"log",     required_argument, 0,  0 },
	    {"compress",      no_argument, 0,  0 },
            {0,         0,                 0,  0 }
        };

        c = getopt_long(argc, argv, "p:l:c",
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
		if (strcmp(name, "log") == 0) {
                    opt_log = 1;
                  if (optarg)
                      arg_log = optarg;
		  logfd = creat(arg_log, 0666);
		  if (logfd < 0) {
		      fprintf(stderr, "Error: the log file could not be created.\n");
		      exit(1);
		  }
                }
		if (strcmp(name, "compress") == 0) {
                    opt_compress = 1;
                }
		break;
		
            case 'p':
		printf("option p\n");
		break;

            case '?':
		fprintf(stderr, "usage: ./lab1b-client [OPTION]...\nvalid option(s): ----port=port, --log=filename, --compress\n");
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
	  int sockfd, portno;
	  struct sockaddr_in serv_addr;
	  struct hostent *server;
//	  char buffer[256];
	  portno = atoi(arg_port);
	  sockfd = socket(AF_INET, SOCK_STREAM, 0);
	  if (sockfd < 0) {
	      fprintf(stderr, "Error opening socket.\n");
	      exit(1);
	  }
	  server = gethostbyname("localhost");
	  if (server == NULL) {
	      fprintf(stderr, "Error opening socket.\n");
              exit(1);
	  }
	  bzero((char *) &serv_addr, sizeof(serv_addr));
	  serv_addr.sin_family = AF_INET;
	  bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	  serv_addr.sin_port = htons(portno);
	  if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) {
	      fprintf(stderr, "Error connecting.\n");
              exit(1);
	  }

	  if (opt_compress == 1) {

	  }
	  
	    signal(SIGPIPE, sigpipe_handler);
	    // Create an array of two pollfd structures, one describing the keyboard (stdin) and one for reading the output returned from  TCP connection with server
	    struct pollfd fds[2];
	    fds[0].fd = 0;
	    fds[0].events = POLLIN;
	    fds[0].revents = 0;
	    fds[1].fd = sockfd;
	    fds[1].events = POLLIN;
	    fds[1].revents = 0;
	    while (!server_exited) {
		int rv_poll = poll(fds, 2, 0);
		if (rv_poll == -1) {
		    fprintf(stderr, "Error polling for events on file descriptors 0 and %d.\npoll: %s\n", sockfd, strerror(errno));
		    exit(1);
		}
		if (rv_poll > 0) {
		    if (fds[0].revents & POLLIN) {
			// Read stdin
			// Read (ASCII) input from the keyboard, echo it to stdout, and forward it to the server
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
				    ssize_t temp = write(1, output, 2);
				    if (temp < 2) {               /* # bytes written may be less than arg count */
					fprintf(stderr, "Error writing to file descriptor 1.\nwrite: %s\n", strerror(errno));
					exit(1);
				    }
				    // Forward to server
				    char c = '\x0A';
				    temp = write(sockfd, &c, 1);
				    if (temp < 1) {              /* # bytes written may be less than arg count */
					fprintf(stderr, "Error writing to file descriptor %d.\nwrite: %s\n", sockfd, strerror(errno));
					exit(1);
				    }
				    if (opt_log)
					print_log(1, logfd, &c, 1);
				}

				else if (input[i] == '\x04') {
				    // Echo to stdout
				    char* str = "^D";
                                    if (write(1, str, 2) < 2) {
                                        fprintf(stderr, "Error writing to file descriptor 1.\nwrite: %s\n", strerror(errno));
                                        exit(1);
                                    }
				}
				else if (input[i] == '\x03') {
				    // Echo to stdout
				    char* str = "^C";
				    if (write(1, str, 2) < 2) {
					fprintf(stderr, "Error writing to file descriptor 1.\nwrite: %s\n", strerror(errno));
					exit(1);
				    }
				}
				else {
				    // Echo to stdout
				    if (input[i] != '\x04' && input[i] != '\x03') {
				    ssize_t temp = write(1, &input[i], 1);
				    if (temp < 1) {              /* # bytes written may be less than arg count */
					fprintf(stderr, "Error writing to file descriptor 1.\nwrite: %s\n", strerror(errno));
					exit(1);
				    }
				    }
				    // Forward to server
				    ssize_t temp = write(sockfd, &input[i], 1);
				    if (temp < 1) {              /* # bytes written may be less than arg count */
					fprintf(stderr, "Error writing to file descriptor %d.\nwrite: %s\n", sockfd, strerror(errno));
					exit(1);
				    }
				    if (opt_log)
					print_log(1, logfd, &input[i], 1);
				}
			    }
			}
		    }
		    if (fds[1].revents & POLLERR || fds[1].revents & POLLHUP) {
			server_exited = 1;
		    }
		    if (fds[1].revents & POLLIN) {
			// Read output from server
			char server_out[512];
			    ssize_t rv_server_out = read(sockfd, &server_out, 511);
			    if (rv_server_out == 0) {     // EOF
				eof_from_shell = 1;
				server_exited = 1;
			    }
			    else if (rv_server_out == -1) {
				fprintf(stderr, "Error reading from file descriptor %d.\nread: %s\n", sockfd, strerror(errno));
				exit(1);
			    }
			    else {
				if (opt_log)
				    print_log(0, logfd, &server_out[0], rv_server_out);
			    for (int i = 0; i < rv_server_out; i++) {
				if (server_out[i] == '\x0A') {
				    char* output = "\x0D\x0A";
				    ssize_t temp = write(1, output, 2);
                                    if (temp < 2) {               /* # bytes written may be less than arg count */
                                        fprintf(stderr, "Error writing to file descriptor 1.\nwrite: %s\n", strerror(errno));
                                        exit(1);
                                    }
				}
				else {
				    ssize_t temp = write(1, &server_out[i], 1);
				    if (temp < 1) {              /* # bytes written may be less than arg count */
                                        fprintf(stderr, "Error writing to file descriptor 1.\nwrite: %s\n", strerror(errno));
                                        exit(1);
                                    }
				}
			    }
			    }
		    }
		}
//	    print_shell_exit_info(pid);
    }
    else {
	fprintf(stderr, "Error: the option --port=port is mandatory.\nusage: ./lab1b-client [OPTION]...\nvalid option(s): ----port=port, --log=filename, --compress\n");
        exit(1);
    }
    exit(0);
}
