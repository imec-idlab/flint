CFLAGS=-std=c11

sink: sink.c sink/conf_parser.c sink/inih/ini.c sink/mqtt_handler.c sock/socket_server.c sink/print.c
	gcc -g -o sink.exe sink.c sink/conf_parser.c sink/inih/ini.c sink/mqtt_handler.c sock/socket_server.c sink/print.c -lpaho-mqtt3c -lcjson -fsanitize=address
	
lora: lora_agent.c sink/conf_parser.c sink/inih/ini.c sink/mqtt_handler.c sock/socket_client.c sink/print.c sink/flint_parser.c
	gcc -g -o lora_agent.exe lora_agent.c sink/conf_parser.c sink/inih/ini.c sink/mqtt_handler.c sock/socket_client.c sink/print.c sink/flint_parser.c -lpaho-mqtt3c -lcjson -fsanitize=address

udp: udp_agent.c sink/conf_parser.c sink/inih/ini.c sink/mqtt_handler.c sock/socket_client.c sink/print.c sink/flint_parser.c
	gcc -g -o udp_agent.exe udp_agent.c sink/conf_parser.c sink/inih/ini.c sink/mqtt_handler.c sock/socket_client.c sink/print.c sink/flint_parser.c -lpaho-mqtt3c -lcjson -fsanitize=address

clean:
	rm sink.exe lora_agent.exe udp_agent.exe

all: sink lora udp