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
#include <mraa.h>
#include <signal.h>
#include <math.h>

int period = 1;
sig_atomic_t volatile run_flag = 1;
mraa_aio_context temp_sensor;
mraa_gpio_context button;

void do_when_interrupted() {
  run_flag = 0;
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

int main(int argc, char ** argv) {

    // Process all arguments
    int c;
    int opt_period = 0;
    int opt_scale = 0;
    int opt_log = 0;
    char default_scale = 'F';
    char * arg_period = NULL;
    char * arg_scale = &default_scale;
    char * arg_log = NULL;

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
                if (optarg)
                    arg_log = optarg;
            }
            break;

        case '?':
            fprintf(stderr, "usage: ./lab4b [OPTION]...\nvalid options: --period=# (default 1), --scale=[FC] (default C), --log=filename\n");
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

    while (run_flag) {
	float temperature = get_temp(arg_scale);
	fprintf(stdout, "%f\n", temperature);
	sleep(1);
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
