#ifndef CONF_PARSER_H
#define CONF_PARSER_H

typedef struct {
    int port;
    const char* ip;
} _socket;

typedef struct {
    const char* id;
    int qos;
    const char* ip;
    int port;
    const char* type;
    const char* destination;
} _sink;

typedef struct {
    const char* name;
    const char* ip;
    const char* protocol;
    int port;
    const char* topic_sub;
    const char* topic_pub;
    int qos;
} _agent;

typedef struct {
    _socket socket;
    _sink sink;
    _agent agent;
} _configuration;

typedef void (*callback)(char* message, int len, char* topic);

int parse_config_file(char* file, _configuration* config);
int config_handler(void* config, const char* section, const char* name, const char* value);
void free_conf_parser(_configuration* config);

#endif