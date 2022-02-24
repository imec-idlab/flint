#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include "mqtt_handler.h"
#include "print.h"

char            mqtt_printd_buf[8192];

void rand_str(char *dest, size_t length) {
    char charset[] = "0123456789"
                     "abcdefghijklmnopqrstuvwxyz"
                     "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    struct timeval time; 
    gettimeofday(&time,NULL);
    srand((time.tv_sec * 1000) + (time.tv_usec / 1000)); // use microseconds as seed when starting multiple applications per second

    while (length-- > 0) {
        size_t index = (double) rand() / RAND_MAX * (sizeof charset - 1);
        *dest++ = charset[index];
    }
    *dest = '\0';
}

void mqtt_delivered(void *context, MQTTClient_deliveryToken dt) {
    mqtt_handle* mq = (mqtt_handle*) context;
    sprintf(mqtt_printd_buf, "Message with token value %d delivery confirmed\n", dt);
    printd(mqtt_printd_buf);
    mq->deliveredtoken = dt;
}

int mqtt_arrvd(void *context, char *topic_name, int topicLen, MQTTClient_message *message) {
    mqtt_handle* mq = (mqtt_handle*) context;

    (*mq->rx_cb)(message->payload, message->payloadlen, topic_name);

    MQTTClient_freeMessage(&message);
    MQTTClient_free(topic_name);
    return 1;
}

void mqtt_connlost(void *context, char *cause) {
    mqtt_handle* mq = (mqtt_handle*) context;
    int rc;
    printe("Connection lost, trying to reconnect ... \n");
    while ((rc = MQTTClient_connect(*mq->mqtt_client, &mq->conn_opts)) != MQTTCLIENT_SUCCESS) {
        sleep(10);
    }
    mqtt_subscribe(mq);
    printe("Connected! \n");
}

int mqtt_publish(char* payload, int payload_len, char* topic, mqtt_handle* mq, int qos) {    
    MQTTClient_deliveryToken token;
    MQTTClient_message pubmsg = MQTTClient_message_initializer;

    pubmsg.payload = payload;
    pubmsg.payloadlen = payload_len;
    pubmsg.qos = qos;
    pubmsg.retained = 0;

    // MQTTResponse res = MQTTClient_publishMessage5(*mqtt_client, topic, &pubmsg, &token);
    int res = MQTTClient_publishMessage(*mq->mqtt_client, topic, &pubmsg, &token);
    if(mq->qos > 0) {
        while (mq->deliveredtoken != token);   
    }
    
    // return res.reasonCode;
    return res;
}

int mqtt_connect(mqtt_handle* mq) {
    int rc, ch;

    mq->mqtt_client = malloc(sizeof(MQTTClient));
    
    mq->clientid = malloc(sizeof(char) * 38);
    rand_str((mq->clientid + 5), 32);
    mq->clientid[0] = 's'; mq->clientid[1] = 'i'; mq->clientid[2] = 'n'; mq->clientid[3] = 'k'; mq->clientid[4] = ':';

    char address[27];
    sprintf(address, "tcp://%s:%d", mq->ip, mq->port);

    MQTTClient_create(mq->mqtt_client, address, mq->clientid, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    
    mq->conn_opts.keepAliveInterval = 20;
    mq->conn_opts.cleansession = 1;
    MQTTClient_setCallbacks(*mq->mqtt_client, mq, &mqtt_connlost, &mqtt_arrvd, &mqtt_delivered);
    if ((rc = MQTTClient_connect(*mq->mqtt_client, &mq->conn_opts)) != MQTTCLIENT_SUCCESS) {
        return rc;
    }

    mq->deliveredtoken = 0;

    return 0;
}

int mqtt_subscribe(mqtt_handle* mq) {
    if(mq->mqtt_client == NULL) {
        return -1;
        printe("failed to subscribe, did you connect?\n");
    }

    MQTTClient_subscribe(*mq->mqtt_client, mq->topic_sub, mq->qos);

    return 0;
}

void free_mqtt_client(mqtt_handle* mq) {
    free(mq->clientid);
    MQTTClient_disconnect(*mq->mqtt_client, 10000);
    MQTTClient_destroy(mq->mqtt_client);
    free(mq->mqtt_client);
}