/*
  NAME: Stewart Dulaney
  EMAIL: sdulaney@ucla.edu
  ID: 904064791
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <mraa.h>
#include <signal.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/poll.h>

// getopt options and arguments
int opt_period = 0;
int opt_scale = 0;
int opt_log = 0;
char scale = 'F';
char * arg_period = NULL;
char * arg_scale = &scale;
char * arg_log = NULL;

int period = 1;
int report = 1;
mraa_aio_context temp_sensor;
mraa_gpio_context button;
struct timeval raw_time;
struct tm* parsed_time;
time_t next_time = 0;
int logfd = 0;
char command_buf[512];
char* command_buf_ptr = &command_buf[0];
int command_buf_capac = 512;

// from_cmd is 1 when shutdown is triggered by a command, and 0 otherwise
void shutdown(int from_cmd) {
    gettimeofday(&raw_time, NULL);
    parsed_time = localtime(&raw_time.tv_sec);
    dprintf(1, "%02d:%02d:%02d SHUTDOWN\n", parsed_time->tm_hour, parsed_time->tm_min, parsed_time->tm_sec);
    if (opt_log) {
	if (from_cmd)
	    dprintf(logfd, "OFF\n");
	dprintf(logfd, "%02d:%02d:%02d SHUTDOWN\n", parsed_time->tm_hour, parsed_time->tm_min, parsed_time->tm_sec);
    }
    exit(0);
}

void do_when_interrupted() {
    shutdown(0);
}

// Precondition: *scale is 'C' or 'F'
float get_temp(char* scale) {
    if (scale == NULL)
	return 0.0;
    int input_voltage = mraa_aio_read(temp_sensor);
    if (input_voltage == -1) {
	fprintf(stderr, "Error reading from AIO 1 (temperature sensor).\n");
	mraa_deinit();
	exit(1);
    }
    const int B = 4275;
    const int R0 = 100000;
    float R = 1023.0/input_voltage-1.0;
    R = R0*R;
    float temperature = 1.0/(log(R/R0)/B+1/298.15)-273.15;
    if (*scale == 'C') {
	return temperature;
    }
    else {
	return temperature * (9.0/5) + 32;
    }
}

void print_report() {
    gettimeofday(&raw_time, NULL);
    if (report && raw_time.tv_sec >= next_time) {
	float temperature = get_temp(arg_scale);
	parsed_time = localtime(&raw_time.tv_sec);
	dprintf(1, "%02d:%02d:%02d %.1f\n", parsed_time->tm_hour, parsed_time->tm_min, parsed_time->tm_sec, temperature);
	if (opt_log)
	    dprintf(logfd, "%02d:%02d:%02d %.1f\n", parsed_time->tm_hour, parsed_time->tm_min, parsed_time->tm_sec, temperature);
	next_time = raw_time.tv_sec + period;
    }
}

void handle_commands(char* cmd, int len) {
    if (strncmp(cmd, "OFF\n", len) == 0) {
	shutdown(1);
    }
    else if (strncmp(cmd, "SCALE=F\n", len) == 0) {
	scale = 'F';
    }
    else if (strncmp(cmd, "SCALE=C\n", len) == 0) {
	scale = 'C';
    }
    else if (strncmp(cmd, "PERIOD=", 7) == 0) {
	char ascii_period[len - 7];
	int i;
	for (i = 0; i < len - 7; i++) {
	    ascii_period[i] = cmd[7 + i];
	}
	const char* ptr = &ascii_period[0];
	period = atoi(ptr);
    }
    else if (strncmp(cmd, "STOP\n", len) == 0) {
	report = 0;
    }
    else if (strncmp(cmd, "START\n", len) == 0) {
	report = 1;
    }
    else if (strncmp(cmd, "LOG ", 4) == 0) {
	// Requires no action beyond logging receipt
    }
    // Log all valid and invalid commands
    if (opt_log) {
	int i;
	for (i = 0; i < len; i++) {
	    dprintf(logfd, "%c", cmd[i]);
	}
    }
}

void process_commands() {
    ssize_t rv = read(0, command_buf_ptr, command_buf_capac);
    
    if (rv == -1) {
	fprintf(stderr, "Error reading STDIN.\n");
	exit(1);
    }
    int cmd_len = 0;
    int i;
    for (i = 0; i < rv + (512 - command_buf_capac); i++) {
	cmd_len++;
	if (command_buf[i] == '\n') {
	    handle_commands(&command_buf[i - cmd_len + 1], cmd_len);
	    cmd_len = 0;
	}
    }

    if (cmd_len > 0) {
	int i;
	for (i = 0; i < cmd_len; i++) {
	    command_buf[i] = command_buf[(rv + (512 - command_buf_capac)) - cmd_len + i + 1];
	}
	command_buf_ptr = &command_buf[0] + cmd_len;
	command_buf_capac = 512 - cmd_len;
	}
}

int main(int argc, char ** argv) {

    // Process all arguments
    int c;

    while (1) {
        int option_index = 0;
        static struct option long_options[] = {
            {
                "period",
                required_argument,
                0,
                0
            },
            {
                "scale",
                required_argument,
                0,
                0
            },
            {
                "log",
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

        c = getopt_long(argc, argv, "p:s:l:",
            long_options, & option_index);
        if (c == -1)
            break;

        const char * name = long_options[option_index].name;
        switch (c) {
        case 0:
            if (strcmp(name, "period") == 0) {
                opt_period = 1;
                if (optarg) {
                    arg_period = optarg;
		    period = atoi(arg_period);
		}
            } else if (strcmp(name, "scale") == 0) {
                opt_scale = 1;
                if (optarg)
                    arg_scale = optarg;
            } else if (strcmp(name, "log") == 0) {
		opt_log = 1;
                if (optarg)
                    arg_log = optarg;
		logfd = creat(arg_log, 0666);
		if (logfd < 0) {
		    fprintf(stderr, "Error: the log file could not be created.\n");
		    exit(1);
		}
            }
            break;

        case '?':
            fprintf(stderr, "usage: ./lab4b [OPTION]...\nvalid options: --period=# (default 1), --scale=[FC] (default F), --log=filename\n");
            exit(1);
            break;

        default:
            printf("?? getopt returned character code 0%o ??\n", c);
        }
    }

    // Initialize AIO pin for temperature sensor
    temp_sensor = mraa_aio_init(1);
    if (temp_sensor == NULL) {
	fprintf(stderr, "Error initializing AIO 1 (temperature sensor).\n");
	mraa_deinit();
	exit(1);
    }

    // Initialize GPIO pin for the button
    button = mraa_gpio_init(60);
    if (button == NULL) {
	fprintf(stderr, "Error initializing GPIO 60 (button).\n");
	mraa_deinit();
	exit(1);
    }

    // Use for MRAA error checking
    int status;

    // Set GPIO button to input
    status = mraa_gpio_dir(button, MRAA_GPIO_IN);
    if (status != MRAA_SUCCESS) {
	fprintf(stderr, "Error setting GPIO 60 (button) to input.\n");
	mraa_deinit();
	exit(1);
    }

    // When the button is pressed, call do_when_interrupted
    mraa_gpio_isr(button, MRAA_GPIO_EDGE_RISING, &do_when_interrupted, NULL);

    struct pollfd poll_commands[1];
    poll_commands[0].fd = 0;
    poll_commands[0].events = POLLIN;
    poll_commands[0].revents = 0;

    while (1) {
	print_report();
	int rv_poll = poll(&poll_commands[0], 1, 0);
	if (rv_poll == -1) {
	    fprintf(stderr, "Error polling for commands on STDIN.\n");
	    exit(1);
	}
	else if (rv_poll > 0 && (poll_commands[0].revents & POLLIN)) {
	    process_commands();
	}
    }

    // Close AIO temperature sensor
    status = mraa_aio_close(temp_sensor);
    if (status != MRAA_SUCCESS) {
	fprintf(stderr, "Error closing AIO 1 (temperature sensor).\n");
	mraa_deinit();
	exit(1);
    }

    // Close GPIO button
    status = mraa_gpio_close(button);
    if (status != MRAA_SUCCESS) {
	fprintf(stderr, "Error closing GPIO 60 (button).\n");
	mraa_deinit();
	exit(1);
    }
    
  
    exit(0);
}
