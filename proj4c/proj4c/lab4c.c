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

#include <sys/socket.h>

#include <netinet/in.h>

#include <netdb.h>

#include <openssl/ssl.h>

#include <openssl/err.h>

#include <openssl/evp.h>

#include <libgen.h>

// getopt options and arguments
int opt_period = 0;
int opt_scale = 0;
int opt_log = 0;
int opt_id = 0;
int opt_host = 0;
char scale = 'F';
char * arg_period = NULL;
char * arg_scale = & scale;
char * arg_log = NULL;
char * arg_id = NULL;
char * arg_host = NULL;

int period = 1;
int report = 1;
int id = 0;
int port = 0;
mraa_aio_context temp_sensor;
struct timeval raw_time;
struct tm * parsed_time;
time_t next_time = 0;
int sockfd = 0;
int logfd = 0;
char command_buf[512];
char * command_buf_ptr = & command_buf[0];
int command_buf_capac = 512;
SSL * conn_data;


void shut_down(int tls) {
    gettimeofday( & raw_time, NULL);
    parsed_time = localtime( & raw_time.tv_sec);
    if (tls) {
        char buf[18];
        snprintf(&buf[0], 18, "%02d:%02d:%02d SHUTDOWN\n", parsed_time->tm_hour, parsed_time->tm_min, parsed_time->tm_sec);
        int ret_code = SSL_write(conn_data, &buf, 18);
        if (ret_code < 18) {
            fprintf(stderr, "Error writing SHUTDOWN with TLS.\n");
            exit(2);
        }
    } else {
        dprintf(sockfd, "%02d:%02d:%02d SHUTDOWN\n", parsed_time->tm_hour, parsed_time->tm_min, parsed_time->tm_sec);
    }
    if (opt_log) {
        dprintf(logfd, "OFF\n");
        dprintf(logfd, "%02d:%02d:%02d SHUTDOWN\n", parsed_time->tm_hour, parsed_time->tm_min, parsed_time->tm_sec);
    }
    exit(0);
}

// Precondition: *scale is 'C' or 'F'
float get_temp(char * scale) {
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
    float R = 1023.0 / input_voltage - 1.0;
    R = R0 * R;
    float temperature = 1.0 / (log(R / R0) / B + 1 / 298.15) - 273.15;
    if ( * scale == 'C') {
        return temperature;
    } else {
        return temperature * (9.0 / 5) + 32;
    }
}

void send_report(int tls) {
    gettimeofday( & raw_time, NULL);
    if (report && raw_time.tv_sec >= next_time) {
        float temperature = get_temp(arg_scale);
        parsed_time = localtime( & raw_time.tv_sec);
        if (tls) {
            char buf[14];
            snprintf(&buf[0], 14, "%02d:%02d:%02d %.1f\n", parsed_time->tm_hour, parsed_time->tm_min, parsed_time->tm_sec, temperature);
            int ret_code = SSL_write(conn_data, &buf, 14);
            if (ret_code < 14) {
                fprintf(stderr, "Error sending report with TLS.\n");
                exit(2);
            }
        } else {
            dprintf(sockfd, "%02d:%02d:%02d %.1f\n", parsed_time->tm_hour, parsed_time->tm_min, parsed_time->tm_sec, temperature);
        }
        if (opt_log)
            dprintf(logfd, "%02d:%02d:%02d %.1f\n", parsed_time->tm_hour, parsed_time->tm_min, parsed_time->tm_sec, temperature);
        next_time = raw_time.tv_sec + period;
    }
}

void handle_commands(char * cmd, int len, int tls) {
    if (strncmp(cmd, "OFF\n", len) == 0) {
        shut_down(tls);
    } else if (strncmp(cmd, "SCALE=F\n", len) == 0) {
        scale = 'F';
    } else if (strncmp(cmd, "SCALE=C\n", len) == 0) {
        scale = 'C';
    } else if (strncmp(cmd, "PERIOD=", 7) == 0) {
        char ascii_period[len - 7];
        int i;
        for (i = 0; i < len - 7; i++) {
            ascii_period[i] = cmd[7 + i];
        }
        const char * ptr = & ascii_period[0];
        period = atoi(ptr);
    } else if (strncmp(cmd, "STOP\n", len) == 0) {
        report = 0;
    } else if (strncmp(cmd, "START\n", len) == 0) {
        report = 1;
    } else if (strncmp(cmd, "LOG ", 4) == 0) {
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

void process_commands(int tls) {
    int rv = 0;
    if (tls) {
        rv = SSL_read(conn_data, command_buf_ptr, command_buf_capac);
        if (rv <= 0) {
            fprintf(stderr, "Error reading commands with TLS.\n");
            exit(2);
        }
    } else {
        rv = read(sockfd, command_buf_ptr, command_buf_capac);
        if (rv == -1) {
            fprintf(stderr, "Error reading commands.\n");
            exit(2);
        }
    }
    
    int cmd_len = 0;
    int i;
    for (i = 0; i < rv + (512 - command_buf_capac); i++) {
        cmd_len++;
        if (command_buf[i] == '\n') {
            handle_commands( & command_buf[i - cmd_len + 1], cmd_len, tls);
            cmd_len = 0;
        }
    }

    if (cmd_len > 0) {
        int i;
        for (i = 0; i < cmd_len; i++) {
            command_buf[i] = command_buf[(rv + (512 - command_buf_capac)) - cmd_len + i + 1];
        }
        command_buf_ptr = & command_buf[0] + cmd_len;
        command_buf_capac = 512 - cmd_len;
    }
}

void send_id(int tls) {
    // Send (and log) an ID terminated with a newline
    char id_str[13] = {'I', 'D', '=', arg_id[0], arg_id[1], arg_id[2], arg_id[3], arg_id[4], arg_id[5], arg_id[6], arg_id[7], arg_id[8], '\n'};
    int ret_code = 0;
    if (tls) {
        ret_code = SSL_write(conn_data, &id_str, 13);
        if (ret_code < 13) {
            fprintf(stderr, "Error writing ID to socket.\n");
            exit(2);
        }
    } else {
        ret_code = write(sockfd, &id_str, 13);
        if (ret_code < 13) {
            fprintf(stderr, "Error writing ID to socket.\n");
            exit(2);
        }
    }
    if (opt_log) {
        ret_code = write(logfd, &id_str, 13);
        if (ret_code < 13) {
            fprintf(stderr, "Error writing ID to log.\n");
            exit(2);
        }
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
                "id",
                required_argument,
                0,
                0
            },
            {
                "host",
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

        c = getopt_long(argc, argv, "p:s:l:i:h:",
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
            } else if (strcmp(name, "id") == 0) {
                opt_id = 1;
                if (optarg)
                    arg_id = optarg;
                if (strlen(arg_id) != 9) {
                    fprintf(stderr, "Error: id must be a 9 digit number.\n");
                    exit(1);
                }
                id = atoi(arg_id);
            } else if (strcmp(name, "host") == 0) {
                opt_host = 1;
                if (optarg)
                    arg_host = optarg;
            }
            break;

        case '?':
            fprintf(stderr, "usage: ./lab4c_tcp [OPTION]...\nvalid options: --period=# (default 1), --scale=[FC] (default F), --log=filename, --id=9-digit-number, --host=name or address\n");
            exit(1);
            break;

        default:
            printf("?? getopt returned character code 0%o ??\n", c);
        }
    }

    if (opt_id == 0) {
        fprintf(stderr, "Error: option --id=9-digit-number is mandatory.\n");
        exit(1);
    }

    if (opt_host == 0) {
        fprintf(stderr, "Error: option --host=name or address is mandatory.\n");
        exit(1);
    }

    if (opt_log == 0) {
        fprintf(stderr, "Error: option --log=filename is mandatory.\n");
        exit(1);
    }

    if (optind == argc - 1) {
        port = atoi(argv[optind]);
        if (port < 0) {
            fprintf(stderr, "Error: invalid port number.\n");
            exit(1);
        }
    }
    else {
        fprintf(stderr, "Error: incorrect number of non-option arguments.\nSingle non-option argument \"port number\" is mandatory.\n");
        exit(1);
    }

    // Use for error checking
    int ret_code = 0;

    // Initialize AIO pin for temperature sensor
    temp_sensor = mraa_aio_init(1);
    if (temp_sensor == NULL) {
        fprintf(stderr, "Error initializing AIO 1 (temperature sensor).\n");
        mraa_deinit();
        exit(1);
    }

    // Open a TCP connection to the server at the specified address and port
    struct sockaddr_in serv_addr;
    struct hostent * server;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        fprintf(stderr, "Error opening socket.\n");
        exit(2);
    }
    server = gethostbyname(arg_host);
    if (server == NULL) {
        fprintf(stderr, "Error getting host info with gethostbyname.\n");
        exit(2);
    }
    bzero((char * ) & serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char * ) server->h_addr, (char * ) & serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(port);
    if (connect(sockfd, (struct sockaddr * ) & serv_addr, sizeof(serv_addr)) < 0) {
        fprintf(stderr, "Error connecting.\n");
        exit(2);
    }

    struct pollfd poll_commands[1];
    poll_commands[0].fd = sockfd;
    poll_commands[0].events = POLLIN;
    poll_commands[0].revents = 0;

    if (strncmp(basename(argv[0]), "lab4c_tcp", 9) == 0) {
        send_id(0);
        while (1) {
            send_report(0);
            int rv_poll = poll( & poll_commands[0], 1, 0);
            if (rv_poll == -1) {
                fprintf(stderr, "Error polling for commands.\n");
                exit(2);
            } else if (rv_poll > 0 && (poll_commands[0].revents & POLLIN)) {
                process_commands(0);
            }
        }
    } else if (strncmp(basename(argv[0]), "lab4c_tls", 9) == 0) {
        // TLS set up
        SSL_load_error_strings();
        SSL_library_init();
        // OPENSSL_add_all_algorithms();
        SSL_CTX * ssl_ctx;
        // ssl_ctx = SSL_CTX_new(TLSv1_client_method());
        ssl_ctx = SSL_CTX_new(TLS_client_method());
        if (ssl_ctx == NULL) {
            fprintf(stderr, "Error creating TLS context object.\n");
            exit(2);
        }
        conn_data = SSL_new(ssl_ctx);
        if (conn_data == NULL) {
            fprintf(stderr, "Error creating TLS connection data struct.\n");
            exit(2);
        }
        if (SSL_set_fd(conn_data, sockfd) == 0) {
            fprintf(stderr, "Error setting TLS file descriptor.\n");
            exit(2);
        }
        ret_code = SSL_connect(conn_data);
        if (ret_code == 0) {
            fprintf(stderr, "Error initiating TLS handshake with the server (controlled shutdown).\n");
            exit(2);
        }
        if (ret_code < 0) {
            fprintf(stderr, "Error initiating TLS handshake with the server (fatal protocol-level error).\n");
            exit(2);
        }

        send_id(1);
        while (1) {
            send_report(1);
            int rv_poll = poll( & poll_commands[0], 1, 0);
            if (rv_poll == -1) {
                fprintf(stderr, "Error polling for commands.\n");
                exit(2);
            } else if (rv_poll > 0 && (poll_commands[0].revents & POLLIN)) {
                process_commands(1);
            }
        }
    }

    // Close AIO temperature sensor
    int status = mraa_aio_close(temp_sensor);
    if (status != MRAA_SUCCESS) {
        fprintf(stderr, "Error closing AIO 1 (temperature sensor).\n");
        mraa_deinit();
        exit(1);
    }

    exit(0);
}