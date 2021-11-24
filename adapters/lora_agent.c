#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#include "sink/conf_parser.h"
#include "sink/mqtt_handler.h"
#include "sock/socket_client.h"
#include "sink/flint_parser.h"
#include "sink/print.h"

#define NUM_ARG     2
#define UUID_LEN    45

udp_client*     sink;
mqtt_handle*    lora_mq;    
char            printd_buf[2048];
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
        sprintf(printd_buf, "%d arguments expected.\n", NUM_ARG);
        printd(printd_buf);
        return -1;
    }
}

void cleanup(_configuration* config, udp_client* cl) {
    free_conf_parser(config);
    if(cl){
        socket_client_stop(cl);
    }
}

/* 
 * received a message from the LoRa network
 */
static void mqtt_callback(char* message, int len, char* topic) {
    cJSON *tree = NULL;
    tree = cJSON_Parse(message);

    if(tree) {
        /* create FLINT format */
        cJSON* flint_tree = flint_build_tree();

        int err = flint_append_device_ctrl_from_chirpstack(flint_tree, tree);
        err |= flint_append_input_ctrl(flint_tree, tree, &config);
        err |= flint_append_adapter_ctrl(flint_tree, &config);
        err |= flint_set_method(flint_tree, POST);

        if (err) {
            sprintf(printd_buf, "appending information failed \n");
            printe(printd_buf);
            return;
        }

        char* json_message = cJSON_PrintUnformatted(flint_tree);

        sprintf(printd_buf, "forwarding message: %s\n", json_message);
        printd(printd_buf);

        socket_client_send(sink, json_message, strlen(json_message));
        free(json_message);

        cJSON_Delete(flint_tree);
    }

    cJSON_Delete(tree);
}

/* 
 * received a message from the FLINT platform
 */
static void socket_receive_callback(char* message, int len, char* topic) {
    cJSON *tree = NULL;
    tree = cJSON_Parse(message);
    if(tree) {
        /* forward to Chirpstack if method is POST */
        enum METHOD method = flint_get_method(tree);
        if(method == POST) {
            cJSON* chirpstack_tree = cJSON_CreateObject();
            if(chirpstack_tree) {
                cJSON* ack = cJSON_CreateFalse();
                cJSON_AddItemToObject(chirpstack_tree, "confirmed", ack);

                cJSON* input_ctrl = cJSON_GetObjectItem(tree, "input-ctrl");
                cJSON* input_custom_ctrl = cJSON_GetObjectItem(input_ctrl, "input-custom-ctrl");

                cJSON* dev_eui = cJSON_GetObjectItem(input_custom_ctrl, "mac");
                char* eui = dev_eui->valuestring;

                cJSON* network_definitions = cJSON_GetObjectItem(input_custom_ctrl, "networkDefinitions");
                cJSON* fport = cJSON_GetObjectItem(network_definitions, "fport");
                cJSON_AddItemToObject(chirpstack_tree, "fPort", fport);

                cJSON* device_ctrl = cJSON_GetObjectItem(tree, "device-ctrl");
                cJSON* data = cJSON_GetObjectItem(device_ctrl, "data");
                cJSON_AddItemToObject(chirpstack_tree, "data", data);

                char top[128];
                sprintf(top, "application/%d/device/%s/command/down", config.agent.application_id, eui);

                char* json_message = cJSON_PrintUnformatted(chirpstack_tree);

                mqtt_publish(json_message, strlen(json_message), top, lora_mq, 0);
                sprintf(printd_buf, "forwarded %s to LoRa network\n", json_message);
                printd(printd_buf);
                
                free(json_message);
            }

            cJSON_Delete(chirpstack_tree);
        } else {
            sprintf(printd_buf, "Did not receive method POST \n");
            printe(printd_buf);
            return;
        } /* end if method == POST */
    }
}

int main(int argc, char* argv[]) {
    // parse arguments
    if(parse_arguments(argc, argv) != 0) {
        return 1;
    }

    char ch;
    printd("starting LoRa agent, press 'ctrl' + 'c' to quit\n");

    // parse the .ini file
    if (parse_config_file(config_file, &config) < 0) {
        sprintf(printd_buf, "can't load %s\n", config_file);
        printe(printd_buf);
        return 1;
    } else {
        sprintf(printd_buf, "loaded config file %s\n", config_file);
        printd(printd_buf);
    }

    // init mqtt client
    mqtt_handle lora = {
        .topic_sub = config.agent.topic_sub,
        .ip = config.agent.ip,
        .port = config.agent.port,
        .qos = config.agent.qos,
        .rx_cb = &mqtt_callback,
        .conn_opts = MQTTClient_connectOptions_initializer
    };

    sprintf(printd_buf, "connecting to LoRa network %s\n", config.agent.ip);
    printd(printd_buf);
    int rc = mqtt_connect(&lora);
    if(!rc) {
        rc = mqtt_subscribe(&lora);
        lora_mq = &lora;
        if(!rc) {
            sprintf(printd_buf, "subscribed to topic %s with qos %d\n", lora.topic_sub, lora.qos);
            printd(printd_buf);
        }
    } else {
        sprintf(printd_buf, "failed to connect, return code %d\n", rc);
        printe(printd_buf);
        cleanup(&config, NULL);
        free_mqtt_client(&lora);
        return 1;
    }

    sink = malloc(sizeof(udp_client));
    sink->socket_cb = &socket_receive_callback;

    rc = sink_client_start(config.socket.ip, config.socket.port, sink);

    while(rc > 0) {
        rc = socket_client_loop(sink);
    }

    cleanup(&config, sink);
    free_mqtt_client(&lora);

    return 0;
}
