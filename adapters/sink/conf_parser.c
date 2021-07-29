#include <stdlib.h>
#include <string.h>

#include "inih/ini.h"
#include "conf_parser.h"

int config_handler(void* config, const char* section, const char* name,
                   const char* value) {
    // config instance for filling in the values.
    _configuration* pconfig = (_configuration*) config;

    // define a macro for checking Sections and keys under the sections.
    #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0

    // fill the values in config struct for Section socket.
    if(MATCH("socket", "socket-port")){
        pconfig->socket.port = atoi(value);
    } else if(MATCH("socket", "socket-ip")){ 
        pconfig->socket.ip = strdup(value);
    }

    // fill the values in config for Section agent.
    if(MATCH("agent", "io-agent-port")){
        pconfig->agent.port = atoi(value);
    } else if(MATCH("agent", "io-agent-ip")){ 
        pconfig->agent.ip = strdup(value);
    } else if(MATCH("agent", "name")){ 
        pconfig->agent.name = strdup(value);
    } else if(MATCH("agent", "protocol")){ 
        pconfig->agent.protocol = strdup(value);
    } else if(MATCH("agent", "sub-topic")){ 
        pconfig->agent.topic_sub = strdup(value);
    } else if(MATCH("agent", "pub-topic")){ 
        pconfig->agent.topic_pub = strdup(value);
    } else if(MATCH("agent", "qos")){
        pconfig->agent.qos = atoi(value);
    } 

    // fill the values in config struct for Section sink.
    else if(MATCH("sink", "id")){
        pconfig->sink.id = strdup(value);
    } else if(MATCH("sink", "qos")) {
        pconfig->sink.qos = atoi(value);
    } else if(MATCH("sink", "broker-ip")){
        pconfig->sink.ip = strdup(value);
    } else if(MATCH("sink", "broker-port")){
        pconfig->sink.port = atoi(value);
    } else if(MATCH("sink", "adapter-type")){
        pconfig->sink.type = strdup(value);
    } else if(MATCH("sink", "adapter-destination")) {
        pconfig->sink.destination = strdup(value);
    } else {
        return 0;
    }

    return 1;
}

int parse_config_file(char* config_file, _configuration* config) {
	return ini_parse(config_file, config_handler, config);
}

void free_conf_parser(_configuration* config) {
    free((void*)config->socket.ip);
    free((void*)config->sink.id);
    free((void*)config->sink.ip);
    free((void*)config->sink.type);
    free((void*)config->sink.destination);
    free((void*)config->agent.name);
    free((void*)config->agent.topic_sub);
    free((void*)config->agent.topic_pub);
    free((void*)config->agent.ip);
    free((void*)config->agent.protocol);
}