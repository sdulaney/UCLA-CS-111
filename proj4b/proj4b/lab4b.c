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

//int period = 1;

sig_atomic_t volatile run_flag = 1;

void do_when_interrupted() {
  run_flag = 0;
}

int main() {

    // Process all arguments
  /*  int c;
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
*/

    mraa_gpio_context buzzer, button;
    buzzer = mraa_gpio_init(62);
    button = mraa_gpio_init(60);

    mraa_gpio_dir(buzzer, MRAA_GPIO_OUT);
    mraa_gpio_dir(button, MRAA_GPIO_IN);

    mraa_gpio_isr(button, MRAA_GPIO_EDGE_RISING, &do_when_interrupted, NULL);

    while (run_flag) {
	mraa_gpio_write(buzzer, 1);
	sleep(1);
	mraa_gpio_write(buzzer, 0);
	sleep(1);
    }

    mraa_gpio_write(buzzer, 0);
    mraa_gpio_close(buzzer);
    mraa_gpio_close(button);
  
    exit(0);
}
