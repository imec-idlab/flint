# 01_direct
This example sets up a local example using three flint components:
1. LoRa adapter - MQTT client listening for packets on a broker (port 8000) and forwarding these to the UDP adapter
2. mosquitto - MQTT broker that serves as message bus
3. UDP adapter - UDP client forwarding traffic to a UDP server

---
**NOTE**

This example requires two MQTT brokers: 1 (port 8000) listening for LoRa traffic (MQTT) and 1 (port 8001) serving as a message bus for the flint system.
---

## Run the example
### Set up southbound interface + middleware
First set up a broker which will simulate the LoRa network (port 8000) and one used as message bus (port 8001):
```
sudo apt-get install mosquitto
mosquitto -p 8000 -d
mosquitto -p 8001 -d
```

Next, go to the `adapters` folder and build the sink and the LoRa agent
```
gcc -o sink.exe sink.c sink/conf_parser.c sink/inih/ini.c sink/mqtt_handler.c sock/socket_server.c sink/print.c -lpaho-mqtt3c -lcjson -fsanitize=address
gcc -o lora_agent.exe lora_agent.c sink/conf_parser.c sink/inih/ini.c sink/mqtt_handler.c sock/socket_client.c sink/flint_parser.c sink/print.c -lpaho-mqtt3c -lcjson -fsanitize=address
```

Run the LoRa sink and the LoRa agent
```
./sink.exe examples/01_direct/lora/conf/adapter_conf.ini DEBUG

> 2021-07-28 10:33:33: starting Sink, press 'ctrl' + 'c' to quit
> 2021-07-28 10:33:33: loaded config file examples/01_direct/lora/conf/adapter_conf.ini
> 2021-07-28 10:33:33: connecting to host 127.0.0.1
> 2021-07-28 10:33:33: subscribed client >sink:zttwVR0fMZkLSvQcdYXL93xNfmEK8UcI to topic urn:uuid:cc0c1e3b-ce09-4c44-b9da-57ee9871497a/in with qos 0
> 2021-07-28 10:33:47: client connected 

```

```
./lora_agent.exe examples/01_direct/lora/conf/adapter_conf.ini DEBUG

> 2021-07-28 10:33:47: starting LoRa agent, press 'ctrl' + 'c' to quit
> 2021-07-28 10:33:47: loaded config file examples/01_direct/lora/conf/adapter_conf.ini
> 2021-07-28 10:33:47: connecting to host 127.0.0.1
> 2021-07-28 10:33:47: subscribed to topic application/+/device/+/event/up with qos 0
> 2021-07-28 10:33:47: connected to 127.0.0.1:10001

```

### Set up northbound interface
First set up a `netcat` UDP server that we consider our service endpoint:
```
nc -u l 5000
```

Again, go to the `adapters` folder and now build the UDP agent
```
gcc -o udp_agent.exe udp_agent.c sink/conf_parser.c sink/inih/ini.c sock/socket_client.c sink/flint_parser.c sink/print.c -lpaho-mqtt3c -lcjson -lqpid-proton -fsanitize=address
```

Run the UDP sink and the UDP agent
```
./sink.exe examples/01_direct/udp/conf/adapter_conf.ini DEBUG

> 2021-07-28 10:33:33: starting Sink, press 'ctrl' + 'c' to quit
> 2021-07-28 10:33:33: loaded config file examples/01_direct/lora/conf/adapter_conf.ini
> ...
```

```
./udp_agent.exe examples/01_direct/udp/conf/adapter_conf.ini DEBUG

> 2021-07-28 10:54:33: starting udp agent, press 'ctrl' + 'c' to quit
> 2021-07-28 10:54:33: loaded config file examples/01_direct/udp/conf/adapter_conf.ini
> 2021-07-28 10:54:33: connected to 127.0.0.1:5000
> 2021-07-28 10:54:33: connected to 127.0.0.1:10002

```

### Send traffic
Now you can start sending traffic using `mosquitto`.
```
sudo apt-get install mosquitto-clients

mosquitto_pub -h 127.0.0.1 -p 8000 -t "application/1/device/8000/event/up" -m '{"devEUI": "8000", "data": "00040x1.83fb035237a27p+30", "fPort": 10, "txInfo": {"dr": 5, "frequency": 868300000}, "rxInfo": [{"gatewayID": "1dee0be9d4472aaf", "uplinkID": "270bb8f5-ccb0-4382-abb6-11e2a9774690", "name": "LoRank8", "rssi": -67, "loRaSNR": 10}]}'
```

You should see the following in the `nc` terminal
```
{"device-ctrl":{"data":"00040x1.83fb035237a27p+30","dev-eui":"0000000000008000","fPort":10,"device-custom-ctrl":{"txInfo":{"dr":5,"frequency":868300000},"rxInfo":[{"gatewayID":"1dee0be9d4472aaf","uplinkID":"270bb8f5-ccb0-4382-abb6-11e2a9774690","name":"LoRank8","rssi":-67,"loRaSNR":10}]}},"input-ctrl":{"input-type":"LoraAdapterAgent","input-ip":"127.0.0.1","input-port":8000,"input-protocol":"mqtt"},"adapter-ctrl":{"from":"urn:uuid:cc0c1e3b-ce09-4c44-b9da-57ee9871497a"}}
```

## Configuration
Every adapter is configured in the conf file in `conf/adapter_conf.ini` of the appropriate agents folder.
Direct adapters do not make use of the Mapper adapter and forward data directly to the final adapter. In this case, the LoRa adapter forwards data directly to the UDP adapter over the message bus (MQTT broker). Direct adapters can be configured by providing the `adapter-destination` in the conf file, e.g.:

```
lora/conf/adapter_conf.ini
...
[sink]
id=urn:uuid:cc0c1e3b-ce09-4c44-b9da-57ee9871497a
broker-ip=127.0.0.1
broker-port=8001
adapter-type=direct
adapter-destination=urn:uuid:f927f1bd-477e-4f99-bc3b-7734d81dd863
...
```

The destination is the `urn` of the UDP adapter
```
...
[sink]
id=urn:uuid:f927f1bd-477e-4f99-bc3b-7734d81dd863
broker-ip=127.0.0.1
broker-port=8001
adapter-type=output
...

```