#ifndef MQTT_HANDLER_H
#define MQTT_HANDLER_H

#include "MQTTClient.h"
#include "conf_parser.h"

typedef struct {
	const char* ip;
	char* clientid;
	int port;
	int qos;
	callback rx_cb;
	MQTTClient* mqtt_client;
	MQTTClient_connectOptions conn_opts;
	const char* topic_sub;
	volatile MQTTClient_deliveryToken deliveredtoken;
} mqtt_handle;

int mqtt_connect(mqtt_handle* mq);
int mqtt_subscribe(mqtt_handle* mq);
int mqtt_publish(char* message, int message_len, char* topic, mqtt_handle* mq, int qos);
void free_mqtt_client();

#endif