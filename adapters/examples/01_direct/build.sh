#!/bin/bash

# build and run the sink
gcc -o sink.exe sink.c sink/conf_parser.c sink/inih/ini.c sink/mqtt_handler.c sock/socket_server.c sink/print.c -lpaho-mqtt3c -lcjson -fsanitize=address
./sink.exe agents/mapper/conf/adapter_conf.ini DEBUG