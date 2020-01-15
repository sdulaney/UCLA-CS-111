/*
 * pty_test ... a program for driving test input through a pty
 *
 *	1. create a pseudo-tty pair
 *	2. start a designated program on the slave side
 *	3. process commands to send and validate input
 *
 * Usage:
 *	pty_test [--verbose] [--script=file] program args ...
 *
 * Command language:
 *	SEND "..."
 *	EXPECT "..."
 *	WAIT [maxwait] (for expectation)
 *	PAUSE waittime
 *	CLOSE	(shut down and collect child status)
 *
 *   if no --script is provided, commands read from stdin
 *
 * Escape conventions in send/expect
 *	graphics	represent themselves
 *	^x		control what ever
 *	\n \r \t	the obvious
 *	^^, \\, \"	literal escapes
 *
 * Output:
 *	everything received goes to stdout
 *	diagnostic output goes to stderr
 *	it exits with the status of the sub-process
 *		... or -1 for bad args or wait timeout
 * TODO
 *	who is leaving orphans, and why
 *	preempt WAIT when EXPECT is satisfied
 *	enable SIGCHLD to gather status
 */

#define	_XOPEN_SOURCE	600	/* pty support	*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <sys/wait.h>
#include <termios.h>

#define MAXARGS	10
#define	BUFSIZE 512

#define	EXIT_BADARG	-1
#define	EXIT_NOCHILD	-1
#define	EXIT_EXPECT	-2

const char *usage = "[-v] [--modes] [--script=file] [--timeout=seconds] [--maxlog=bytes] [--rate=cps] program [args ...]";

int logged = 0;
int maxlog = 10000;
int timeout = 0;
int verbose = 0;
int modes = 0;
int rate = 10;

/* ptys come in master/slave pairs	*/
struct pty_fds {
	int	master;
	int 	slave;
} fds;

/* communication between command and output threads	*/
char *expecting = 0;	/* expecting input		*/
int writer_stop = 0;	/* shut down 			*/
pid_t child_pid = 0;	/* child process 		*/
int child_status = -1;	/* child exit status		*/

/* 
 * signal handler ... just for wakeups
 * 	just return
 */
void handler( int sig ) {
}

/*
 * alarm handler ... this is taking too long
 */
void time_handler( int sig ) {
	writer_stop = 1;
	fprintf(stderr, "!!! ALARM !!! Killing Child process %d\n", child_pid);
	kill(child_pid, SIGTERM);
}

/*
 * sigchild handler ... collect status
 */
void child_handler( int sig ) {
	if (verbose)
		fprintf(stderr, "\n... SIGCHLD: %d has exited\n", child_pid);
	if (child_status == -1) {
		waitpid(child_pid, &child_status, 0);
		if (verbose)
			fprintf(stderr, "... exit status: %d\n", child_status);
	} else if (verbose) 
		fprintf(stderr, "... status already collected: %d\n", child_status);
}

/*
 * process_output ... copy data coming back from pty to stdout
 *
 * @return	0: normal shutdown
 *		-1: unexpected read error from pty
 */
void *process_output( void *unused ) {
	char inbuf[BUFSIZE];
	int infd = fds.master;

	uintptr_t ret = 0;
	signal(SIGTERM, handler);

	/* read until their ain't no more	*/
	for(;;) {
		if (verbose)
			fprintf(stderr, "... writer TOP OF LOOP stop=%d, sts=%d\n", 
				writer_stop, child_status);
		int cnt = read(infd, inbuf, sizeof inbuf);
		if (verbose)
			fprintf(stderr, "... writer cnt=%d\n", cnt);
		if (cnt <= 0) {
			if (verbose)
				fprintf(stderr, "... output thread got EOF\n");
			if (!writer_stop)
				ret = -1;
			break;
		}

		/* tick off expected characters		*/
		if (expecting) {
			char *s = inbuf;
			while( *expecting && s < &inbuf[cnt] ) {
				if (*s++ == *expecting)
					expecting++;
			}

			/* see if we are done	*/
			if (*expecting == 0) {
				expecting = 0;
				if (verbose)
					fprintf(stderr, "... expectation satisfied\n");
			}
		}

		/* and forward all output to stdout	*/
		int rc = write(1, inbuf, cnt);

		/* logging too much output could indicate a run-away child	*/
		logged += cnt;
		if (maxlog > 0 && maxlog < logged) {
			fprintf(stderr, "ERROR ... maximum log size (%d) exceeded, killing sub-process\n", maxlog);
			kill(child_pid, SIGTERM);
			break;
		}
	}
	
	pthread_exit((void *) ret);
}

/*
 * skip leading whitespace in a null terminated string
 */
const char *skipWhite( const char *s ) {
	while( *s == ' ' || *s == '\t' || *s == '\n')
		s++;
	return(s);
}

/*
 * canonize ... interpret escape sequences in a string
 *
 *	string is either quote or space delimited
 *	special characters are \r \n \t or ^letter
 #	a backslash escapes anything else
 *
 *	since we don't expect to send nulls, both
 *	input and output strings are null terminated.
 *
 * @parmam	source string
 * @param	destination buffer
 * 
 * @return	character after source string
 */
const char *canonize( const char *src, char *dest ) {
	char quote;
	if (*src == '\'' || *src == '"')
		quote = *src++;
	else
		quote = ' ';

	while(*src && *src != '\n' && *src != quote) {
		if (*src == '^') {
			*dest++ = src[1] - '@';
			src += 2;
		} else if (*src == '\\') {
			if (src[1] == 'r')
				*dest++ = '\r';
			else if (src[1] == 'n')
				*dest++ = '\n';
			else if (src[1] == 't')
				*dest++ = '\t';
			else
				*dest++ = src[1];
			src += 2;
		} else
			*dest++ = *src++;
	}
	*dest++ = 0;

	return( (*src == quote) ? &src[1] : src );
}

/*
 * escape ... oppositie of canonize
 *		interpret non-graphics
 */
const char *escape( const char *input ) {
	static char outbuf[BUFSIZE];

	char *p = outbuf;
	while( *input ) {
		if (*input < ' ') {
			*p++ = '^';
			*p++ = *input++ + '@';
		} else if (*input == 0x7f) {
			*p++ = '\\';
			*p++ = '1';
			*p++ = '7';
			*p++ = '7';
			input++;
		} else
			*p++ = *input++;
	}
	*p++ = 0;

	return outbuf;
}


/*
 * log the input/output/local termio modes
 */
void showModes(int fd, const char*when) {
	struct termios myModes;
	if (tcgetattr(fd, &myModes) != 0) 
		fprintf(stderr, "Unable to get modes from fd %d: %s\n",
			fd, strerror(errno));
	else {
		long iflg = myModes.c_iflag;	/* poorly typed	*/
		long oflg = myModes.c_oflag;	/* poorly typed	*/
		long lflg = myModes.c_lflag;	/* poorly typed	*/
		fprintf(stderr, "TTY modes(%d,%s): 0x%lx 0x%lx 0x%lx\n",
			fd, when, iflg, oflg, lflg);
	}
}

/*
 * process_command ... drive pty input
 *
 * @param command to be executed
 *
 * @return (boolean) continue processing
 */
int process_command(const char *cmd) {
	static char sendbuf[BUFSIZE];
	static char expbuf[BUFSIZE];

 	if (strncmp("SEND", cmd, 4) == 0) {
		const char *s = skipWhite(&cmd[4]);
		s = canonize(s, sendbuf);
		if (rate == 0) { // unmetered output
			if (verbose)
				fprintf(stderr, "... SEND \"%s\"\n", escape(sendbuf));
			write(fds.master, sendbuf, strlen(sendbuf));
		} else {	// metered output rate
			const char *s = sendbuf;
			int cnt = strlen(sendbuf);
			while(cnt-- > 0) {
				if (verbose) {
					char echobuf[2];
					echobuf[0] = *s;
					echobuf[1] = 0;
					fprintf(stderr, "... SEND \"%s\"\n", escape(echobuf));
				}
				write(fds.master, s++, 1);
				usleep(1000000/rate);
			}
		}
		/* if we have sent too much data, something might be wrong */
		logged += strlen(sendbuf);
		if (maxlog > 0 && maxlog < logged) {
			fprintf(stderr, "ERROR ... maximum log size (%d) exceeded, killing sub-process\n", maxlog);
			kill(child_pid, SIGTERM);
			exit(EXIT_EXPECT);
		}
		return(1);
	} else if (strncmp("EXPECT", cmd, 6) == 0) {
		const char *s = skipWhite(&cmd[6]);
		s = canonize(s, expbuf);
		expecting = expbuf;
		if (verbose)
			fprintf(stderr, "... EXPECT \"%s\"\n", escape(expbuf));
		return(1);
	} else if (strncmp("WAIT", cmd, 4) == 0) {
		const char *s = skipWhite(&cmd[4]);
		int delay = atoi(s);
		if (delay > 0) {
			if (verbose)
				fprintf(stderr, "... WAIT %ds\n", delay);
			if (expecting) {
				sleep(delay);
				if (expecting) {
					fprintf(stderr, "EXPECTATION NOT FULFILLED\n");
					kill(child_pid, SIGTERM);
					exit(EXIT_EXPECT);
				}
			} else if (verbose)
				fprintf(stderr, "... expectation already fulfilled\n");
		}
		return(1);
	} else if (strncmp("PAUSE", cmd, 5) == 0) {
		const char *s = skipWhite(&cmd[5]);
		int delay = atoi(s);
		if (delay > 0) {
			if (verbose)
				fprintf(stderr, "... PAUSE %ds\n", delay);
			sleep(delay);
		}
		return(1);
	} else if (strncmp("MODES", cmd, 5) == 0) {
		const char *s = skipWhite(&cmd[5]);
		char *p;
		for (p = sendbuf; *s && *s != ' ' && *s != '\t' && *s != '\n'; *p++ = *s++);
		*p = 0;
		showModes(fds.master, sendbuf);
		return(1);
	} else if (strncmp("CLOSE", cmd, 5) == 0) {
		int rc = waitpid(child_pid, &child_status, 0);
		fprintf(stderr, "... waitpid returns %d, status=%d\n", rc, child_status);
		return( 0 );
	}

	fprintf(stderr, "Unrecognized command: %s\n", cmd);
		return( 0 );
}

/*
 * process arguments, open the ptys, spawn process at 
 * the other end, and run the main data-passing loop
 */
int main( int argc, char **argv ) {
	/* process initial arguments for me	*/
	char *script = 0;
	int argx;
	for( argx = 1; argx < argc; argx++ ) {
		char *s = argv[argx];
		if (s[0] == '-') {
			if (s[1] == 'v' || strcmp(s, "--verbose") == 0) {
				verbose = 1;
				continue;
			}
			if (s[1] == 'm' || strcmp(s, "--modes") == 0) {
				modes = 1;
				continue;
			}
			if (s[1] == 's') {
				script = s[2] ? &s[2] : argv[++argx];
				continue;
			}
			if (strncmp(s, "--script", 8) == 0) {
				script = (s[8] == '=') ? &s[9] : argv[++argx];
				continue;
			}
			if (s[1] == 't') {
				timeout = atoi( s[2] ? &s[2] : argv[++argx]);
				continue;
			}
			if (strncmp(s, "--timeout", 9) == 0) {
				timeout = atoi(( s[9] == '=') ? &s[10] : argv[++argx]);
				continue;
			}
			if (strncmp(s, "--maxlog", 8) == 0) {
				maxlog = atoi(( s[8] == '=') ? &s[9] : argv[++argx]);
				continue;
			}
			if (s[1] == 'r') {
				rate = atoi( s[2] ? &s[2] : argv[++argx]);
				continue;
			}
			if (strncmp(s, "--rate", 6) == 0) {
				rate = atoi(( s[6] == '=') ? &s[7] : argv[++argx]);
				continue;
			}
			else {
				fprintf(stderr, "Unrecognized argument: %s\n", s);
				fprintf(stderr, "Usage: %s %s\n", argv[0], usage);
				exit(EXIT_BADARG);
			}
		} else 
			break;
	}
	
	if (verbose) {
		fprintf(stderr, "%s ", argv[0]);
		if (verbose) fprintf(stderr, "--verbose ");
		if (modes) fprintf(stderr, "--modes ");
		if (script) fprintf(stderr, "--script=%s ", script);
		if (rate != 0) fprintf(stderr, "--rate=%d ", rate);
		if (timeout != 0) fprintf(stderr, "--timeout=%d ", timeout);
		if (maxlog != 0) fprintf(stderr, "--maxlog=%d ", maxlog);
		fprintf(stderr, "\n");
	}

	/* make sure we have a program to run */
	if (argx >= argc) {
		fprintf(stderr, "No program specified\n");
				fprintf(stderr, "Usage: %s %s\n", argv[0], usage);
				exit(EXIT_BADARG);
	}

	/* switch over to the script	*/
	if (script) {
		int fd = open(script, 0);
		if (fd < 0) {
			fprintf(stderr, "unable to open script %s: %s\n",
				script, strerror(errno));
			exit(EXIT_BADARG);
		}
		close(0);
		dup(fd);
		close(fd);
	}

	/* create the pty	*/
	void get_ptys(struct pty_fds *);
	get_ptys(&fds);
	if (modes)
		showModes(fds.master, "START");

	/* figure out what program we are supposed to start */
	const char *args[MAXARGS];
	int nargs = 0;
	const char *program = argv[argx];
	while( argx < argc && nargs < MAXARGS - 1) {
		args[nargs++] = argv[argx++];
	}
	args[nargs] = 0;

	/* create a new process group, so we can kill it later */
	setpgrp();

	/* kick it off in a sub-process	*/
	child_pid = fork();
	if (child_pid < 0) {
		fprintf(stderr, "Unable to fork child process: %s\n",
			strerror(errno));
		exit(EXIT_NOCHILD);
	} else if (child_pid == 0) {
		if (verbose) {
			fputs("... EXEC ", stderr);
			int i;
			for( i = 0; i < nargs; i++ ) {
				fputs(args[i], stderr);
				fputc(' ', stderr);
			}
			fputc('\n', stderr);
		}
		/* turn pty slave into stdin-stderr for the child	*/
		close(0);
		close(1);
		close(2);
		dup(fds.slave);
		dup(fds.slave);
		dup(fds.slave);

		/* close the original master/slave FDs	*/
		close(fds.slave);
		close(fds.master);

		/* and kick off the specified program	*/
		int ret = execv(program, (char * const *)args);
		fprintf(stderr, "Unable to exec %s: %s\n",
			program, strerror(errno));
		exit(EXIT_NOCHILD);
	} else {
		close(fds.slave);
		if (verbose)
			fprintf(stderr, "... started sub-process %d\n", child_pid);
	}

	/* start thread to pass output from pty to stdout	*/
	pthread_t output_thread;
	if (verbose)
		fprintf(stderr, "... starting output thread\n");
	int rc = pthread_create(&output_thread, NULL, process_output, NULL);
	if (rc < 0) {
		fprintf(stderr, "Unable to create output thread: %s\n",
			strerror(errno));
		kill(child_pid, SIGTERM);
		exit(EXIT_NOCHILD);
	}

	/* prepare for signals		*/
	if (timeout) {
		signal(SIGALRM, time_handler);
		alarm(timeout);
	}
	signal(SIGTERM, handler);
	signal(SIGCHLD, child_handler);

	/* process the commands from stdin to the pty	*/
	char inbuf[BUFSIZE];
	if (verbose)
		fprintf(stderr, "... starting command loop\n");

	while( !feof(stdin) ) {
		if (fgets(inbuf, sizeof inbuf, stdin) == NULL )
			continue;

		/* ignore blank and comment lines */
		const char *s = skipWhite(inbuf);
		if (*s == '#' || *s == '\n' || *s == 0)
			continue;

		if (!process_command(s))
			break;
	}
	if (verbose)
		fprintf(stderr, "... command loop has exited\n");

	/* shut down output thread	*/
	int status;
	writer_stop = 1;
	pthread_kill(output_thread, SIGTERM);
	pthread_join(output_thread, (void **) &status);
	if (verbose)
		fprintf(stderr, "... output thread exited w/status %d\n", status);

	/* report on child process exit status	*/
	if (child_pid != 0 && child_status == -1) {
		fprintf(stderr, "!!! CHILD (%d) STATUS NOT ALREADY COLLECTED\n", child_pid);
		waitpid(child_pid, &child_status, 0);
	}
		
	fprintf(stderr, "%s EXIT SIGNAL=%d, STATUS=%d\n", 
		program, child_status & 0xff, child_status >> 8);

	if (modes)
		showModes(fds.master, "END");

#ifdef MAYBE_LATER
	/* kill anything that may still survive under us	*/
	kill(0, SIGTERM);
#endif

	exit(0);
}

/*
 * get_ptys ... allocate and open a pair of ptys
 *	exits in case of error
 */	
void get_ptys( struct pty_fds *p ) {
	p->master = posix_openpt(O_RDWR);
	if (p->master < 0) {
		fprintf(stderr, "Unable to open master pty: %s\n",
			strerror(errno));
		exit(EXIT_NOCHILD);
	}

	int rc = grantpt(p->master);
	if (rc != 0) {
		fprintf(stderr, "Unable to grant master pty: %s\n",
			strerror(errno));
		exit(EXIT_NOCHILD);
	}
	rc = unlockpt(p->master);
	if (rc != 0) {
		fprintf(stderr, "Unable to unlock master pty: %s\n",
			strerror(errno));
		exit(EXIT_NOCHILD);
	}

	p->slave = open(ptsname(p->master), O_RDWR);
}
