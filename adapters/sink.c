#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <cjson/cJSON.h>

#include "sink/conf_parser.h"
#include "sink/mqtt_handler.h"
#include "sock/socket_server.h"
#include "sink/print.h"

#define NUM_ARG     2
#define UUID_LEN    45

udp_server*     serv;
mqtt_handle*    sink_mq;    
bool            done = false;
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

void cleanup(_configuration* config, udp_server* udps) {
    free_conf_parser(config);
    if(udps) {
        socket_server_stop(udps);
    }
}

static void mqtt_callback(char* message, int len, char* topic) {
    sprintf(printd_buf, "received message on topic %s \n", topic);
    printd(printd_buf);

    cJSON *tree = NULL;
    tree = cJSON_ParseWithLength(message, len);

    if(tree) {
        // todo validate json structure
        char* json_message = cJSON_PrintUnformatted(tree);
        int rc = socket_server_send(serv, json_message, strlen(json_message));
        free(json_message);
    }

    cJSON_Delete(tree);
}

void copy_topic(char* dst, const char* uuid, char* suffix, int suffix_len) {
    for(int i = 0; i < UUID_LEN; i++) {
        dst[i] = uuid[i];
    }
    int j = 0;
    for(int i = UUID_LEN; i < (UUID_LEN + suffix_len); i++) {
        dst[i] = suffix[j];
        j++;
    }
}

int publish_next_hops(cJSON* tree, char* message) {
    int count = 0; char hop[UUID_LEN + 5];
    cJSON* adapter = cJSON_GetObjectItem(tree, "adapter-ctrl");
    if(adapter) {
        if(cJSON_GetObjectItem(adapter, "adapter-scheme")) {
            cJSON* shackle;
            cJSON* scheme = cJSON_GetObjectItem(adapter, "adapter-scheme");
            cJSON_ArrayForEach(shackle, scheme) {
                cJSON* self = cJSON_GetObjectItem(shackle, config.sink.id);
                if(self) {
                    copy_topic(hop, self->valuestring, "/in\0", 4);
                    mqtt_publish(message, strlen(message), hop, sink_mq, sink_mq->qos);

                    sprintf(printd_buf, "forwarding message on topic %s\n", hop);
                    printd(printd_buf);

                    count++;
                }
            }
        } else {
            copy_topic(hop, config.sink.id, "/out\0", 5);
            mqtt_publish(message, strlen(message), hop, sink_mq, sink_mq->qos);
            
            sprintf(printd_buf, "forwarding message on topic %s\n", hop);
            printd(printd_buf);

            count = 1;
        }
    }
    return count;
}

static void socket_receive_callback(char* message, int len, char* topic) {
    cJSON *tree = NULL;
    if(!is_socket_connected) {
        tree = cJSON_Parse(message);
        if(cJSON_GetObjectItem(tree, "sink-ctrl")) {
            printd("client connected \n");
            is_socket_connected = 1;
        }
    } else {
        sprintf(printd_buf, "received message on socket: %s\n", message);
        printd(printd_buf);
        int number_of_hops = 0;
        
        if(!strcmp(config.sink.type, "direct")) {
            number_of_hops = 1;
            char hop[UUID_LEN + 4];
            copy_topic(hop, config.sink.destination, "/in\0", 4);
            mqtt_publish(message, strlen(message), hop, sink_mq, sink_mq->qos);
        } else {
            // get next hop(s) from adapter scheme
            tree = cJSON_Parse(message);
            number_of_hops = publish_next_hops(tree, message);
        }
    }

    cJSON_Delete(tree);
}

int main(int argc, char* argv[]) {
    // parse arguments
    if(parse_arguments(argc, argv) != 0) {
        return 1;
    }

    char ch;
    printe("starting Sink, press 'ctrl' + 'c' to quit\n");

    // parse the .ini file
    if (parse_config_file(config_file, &config) < 0) {
        sprintf(printd_buf, "can't load %s\n", config_file);
        printe(printd_buf);
        return 1;
    } else {
        sprintf(printd_buf, "loaded config file %s\n", config_file);
        printe(printd_buf);
    }

    // init mqtt client
    char topic[64];
    if(!strcmp(config.sink.type, "mapper")) {
        sprintf(topic, "+/out");
    } else {
        sprintf(topic, "%s/in", config.sink.id);
    }

    mqtt_handle sink = {
        .topic_sub = topic,
        .ip = config.sink.ip,
        .port = config.sink.port,
        .qos = config.sink.qos,
        .rx_cb = &mqtt_callback,
        .conn_opts = MQTTClient_connectOptions_initializer
    };

    sprintf(printd_buf, "connecting to MQTT Message Bus on %s\n", config.sink.ip);
    printe(printd_buf);

    int rc = mqtt_connect(&sink);
    if(!rc) {
        rc = mqtt_subscribe(&sink);
        sink_mq = &sink;
        if(!rc) {
            sprintf(printd_buf, "subscribed client %s to topic %s with qos %d\n", sink.clientid, sink.topic_sub, config.sink.qos);
            printe(printd_buf);
        }
    } else {
        sprintf(printd_buf, "failed to connect, return code %d\n", rc);
        printe(printd_buf);
        cleanup(&config, NULL);
        return 1;
    }

    udp_server* serv = malloc(sizeof(udp_server));
    serv->socket_cb = &socket_receive_callback;

    rc = socket_server_start(config.socket.ip, config.socket.port, serv);

    while(rc > 0) {
        rc = socket_server_loop(serv);
    }

    cleanup(&config, serv);
    free_mqtt_client(&sink);

    return 0;
}
