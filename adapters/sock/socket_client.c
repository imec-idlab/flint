#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <unistd.h>

#include "socket_client.h"
#include "../sink/print.h"

char                    printd_buf[2048];

int socket_client_send(udp_client* cl, char* message, int len) {
    int rc, server_address_size;
    
    server_address_size = sizeof(cl->server);

    rc = send(cl->s, message, len, 0);

    return rc;
}

int socket_client_loop(udp_client* cl) {
    int len, server_address_size;
    server_address_size = sizeof(cl->server);

    len = recv(cl->s, cl->buf, cl->buf_len, 0); // block
    if(len < 0) {
        printe("can not load message, maybe check your connection? \n");
        return len;
    }
    if(len < cl->buf_len){
        cl->buf[len] = '\0';
    }

    (*cl->socket_cb)(cl->buf, len, "");

    return 0;
}

int socket_client_stop(udp_client* cl) {
    close(cl->s);
    free(cl->buf);
}

int sink_client_start(const char* ip, const int port, udp_client* cl) {
    int s = socket_client_start(ip, port, cl);
    if(s < 0) {
        return s;
    }

    char msg[128]; 
    sprintf(msg, "{ \"sink-ctrl\": { \"socket\": {\"ip\": \"%s\", \"port\": %d }}}", ip, port);
    if(socket_client_send(cl, msg, strlen(msg)) < 0){
        printe("failed to send message over local socket\n");
        return -1;
    }

    return cl->s;
}

int socket_client_start(const char* ip, const int port, udp_client* cl) {
    int s, namelen;

    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        printe("failed to set up socket\n");
        return -1;
    }

    memset((char *) &cl->server, 0, sizeof(cl->server));

    cl->server.sin_family = AF_INET;
    cl->server.sin_port = htons(port);

    if (inet_aton(ip , &cl->server.sin_addr) == 0) {
        printe("inet_aton() failed\n");
        return -2;
    }

    if(connect(s, (struct sockaddr *) &cl->server, sizeof(struct sockaddr)) < 0) {
        printe("Failed to connect to remote server!\n");
        return -3;
    }

    sprintf(printd_buf, "socket client connected to %s:%d\n", ip, port);
    printd(printd_buf);

    cl->buf_len = 4096;
    cl->buf = (char*) malloc(sizeof(char) * cl->buf_len);
    cl->s = s;

    return s;
}