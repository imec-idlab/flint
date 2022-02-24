#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h> //for threading , link with lpthread

#include "sink/conf_parser.h"
#include "sock/socket_client.h"
#include "sink/print.h"

#define NUM_ARG     2
#define UUID_LEN    45

udp_client*     udp;
udp_client*     sink;
char            udp_print_buf[2048];
char*           config_file = NULL;
int             is_socket_connected = 0;
char            sink_id[UUID_LEN];
_configuration  config;

int parse_arguments(int argc, char* argv[]) {
    if( argc == (NUM_ARG + 1) ) {
        config_file = argv[1];
        if(!strcmp(argv[2], "DEBUG")) {
            set_log_level(1);
        }
        return 0;
    }
    else if( argc > (NUM_ARG + 1) ) {
        printd("Too many arguments supplied.\n");
        return -1;
    }
    else {
        sprintf(udp_print_buf, "%d arguments expected.\n", NUM_ARG);
        printd(udp_print_buf);
        return -1;
    }
}

void cleanup(_configuration* config, udp_client* cl) {
    free_conf_parser(config);
    socket_client_stop(cl);

    free(cl);
}

static void socket_receive_callback(char* message, int len, char* topic) {
    sprintf(udp_print_buf, "received message on socket: %s\n", message);
    printd(udp_print_buf);

    int rc = socket_client_send(udp, message, len); // simply forward to udp server
}

static void udp_receive_callback(char* message, int len, char* topic) {
    sprintf(udp_print_buf, "received message on udp socket: %s\n", message);
    printd(udp_print_buf);
}

int main(int argc, char* argv[]) {
    // parse arguments
    if(parse_arguments(argc, argv) != 0) {
        return 1;
    }

    char ch;
    printd("starting udp agent, press 'ctrl' + 'c' to quit\n");

    // parse the .ini file
    if (parse_config_file(config_file, &config) < 0) {
        sprintf(udp_print_buf, "can't load %s\n", config_file);
        printe(udp_print_buf);
        return 1;
    } else {
        sprintf(udp_print_buf, "loaded config file %s\n", config_file);
        printd(udp_print_buf);
    }

    // connect to sink
    udp = malloc(sizeof(udp_client));
    udp->socket_cb = &udp_receive_callback;

    sink = malloc(sizeof(udp_client));
    sink->socket_cb = &socket_receive_callback;

    socket_client_start(config.agent.ip, config.agent.port, udp);
    sink_client_start(config.socket.ip, config.socket.port, sink);

    while(1) {
        int rc;
        rc = socket_client_loop(sink);
        if(rc < 0) {
            break;
        }
    }

    cleanup(&config, sink);
    socket_client_stop(udp);
    free(udp);

    return 0;
}