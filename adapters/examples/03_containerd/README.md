# 03_containerd
This example sets up a local example using four containerized flint components:
1. LoRa adapter - MQTT client listening for packets on a broker (port 8000) and forwarding these to the UDP adapter
2. mosquitto - MQTT broker that serves as message bus
3. Mapper adapter - Mapper instance forwarding traffic between flint components
4. UDP adapter - UDP client forwarding traffic to a UDP server


## Build Docker containers
First install `Docker` using the provided bootscript
```
sudo bash ./bootscript.sh
```

Every adapter requires a Docker file in order to build a containerized application. These are provided in the `adapters` folder.
Inside that folder, the script `push_docker_registry.sh` will build the Docker containers and push them your local Docker registry (port 50000).

## Run the Docker example
First set up a broker which will simulate the LoRa network (port 8000) and one used as message bus (port 8001):
```
sudo apt-get install mosquitto
mosquitto -p 8000 -d
mosquitto -p 8001 -d
```
### the Southbound interface
Once the containers are pushed to the local registry, the following command can be used to run them locally. *Change the path (using the `-v` flag) to the absolute path were the configuration files are located.*

```
sudo docker run --rm -it --name=LoRaSink --network host -v /home/../flint/adapters/examples/03_containerd/lora/conf/adapter_conf.ini:/usr/src/app/sink/conf/adapter_conf.ini -e config=sink/conf/adapter_conf.ini -e loglevel=DEBUG localhost:50000/flint:sink
```
```
sudo docker run --rm -it --name=LoRaAgent --network host -v /home/../flint/adapters/examples/03_containerd/lora/conf/adapter_conf.ini:/usr/src/app/agents/lora/conf/adapter_conf.ini -e config=agents/lora/conf/adapter_conf.ini -e loglevel=DEBUG localhost:50000/flint:lora-adapter
```

### the Mapper middleware
First, run the Mapper sink:
```
sudo docker run --rm -it --name=MapperSink --network host -v /home/.../flint/adapters/examples/03_containerd/mapper/conf/adapter_conf.ini:/usr/src/app/sink/conf/adapter_conf.ini -e config=sink/conf/adapter_conf.ini -e loglevel=DEBUG localhost:50000/flint:sink
```

Next, run the Mapper agent. Simply copy the complete `mapper` folder to the container to inject thing descriptions, adapter definitions and configuration files.
```
sudo docker run --rm -it --name=MapperAdapter --network host -v /home/../flint/adapters/examples/03_containerd/mapper:/usr/src/app/mapper -e loglevel=DEBUG localhost:50000/flint:mapper
```

### the Northbound interface
First, run the UDP sink:
```
sudo docker run --rm -it --name=UdpSink --network host -v /home/.../flint/adapters/examples/03_containerd/udp/conf/adapter_conf.ini:/usr/src/app/sink/conf/adapter_conf.ini -e config=sink/conf/adapter_conf.ini -e loglevel=DEBUG localhost:50000/flint:sink
```

Next, run the UDP agent:
```
sudo docker run --rm -it --name=UdpAgent --network host -v /home/.../flint/adapters/examples/03_containerd/udp/conf/adapter_conf.ini:/usr/src/app/agents/udp/conf/adapter_conf.ini -e config=agents/udp/conf/adapter_conf.ini -e loglevel=DEBUG localhost:50000/flint:udp-adapter
```

Finally, open a `nc` window:

```
nc -u -l 5000
```

Send MQTT traffic using `mosquitto`:
```
mosquitto_pub -h 127.0.0.1 -p 8000 -t "application/1/device/8000/event/up" -m '{"devEUI": "8000", "data": "00040x1.83fb035237a27p+30", "fPort": 10, "txInfo": {"dr": 5, "frequency": 868300000}, "rxInfo": [{"gatewayID": "1dee0be9d4472aaf", "uplinkID": "270bb8f5-ccb0-4382-abb6-11e2a9774690", "name": "LoRank8", "rssi": -67, "loRaSNR": 10}]}'
```

And you should see the following output in your `nc`:
```
{"device-ctrl":{"data":"00040x1.83fb035237a27p+30","dev-eui":"0000000000008000","fPort":10,"device-custom-ctrl":{"txInfo":{"dr":5,"frequency":868300000},"rxInfo":[{"gatewayID":"1dee0be9d4472aaf","uplinkID":"270bb8f5-ccb0-4382-abb6-11e2a9774690","name":"LoRank8","rssi":-67,"loRaSNR":10}]}},"input-ctrl":{"input-type":"LoraAdapterAgent","input-ip":"127.0.0.1","input-port":8000,"input-protocol":"mqtt"},"adapter-ctrl":{"from":"urn:uuid:4c960148-31ac-4de7-bada-918a99bdc29f","adapter-scheme":[{"urn:uuid:4c960148-31ac-4de7-bada-918a99bdc29f":"urn:uuid:e4370d7c-eae3-416d-85f0-751d396cacf5"},{"urn:uuid:e4370d7c-eae3-416d-85f0-751d396cacf5":"urn:uuid:f927f1bd-477e-4f99-bc3b-7734d81dd863"}]},"output-ctrl":{"ipv6":"0:0:0:0:0:0:0:0","thing-id":"urn:uuid:9b70666c-ebae-428a-a13a-6fdaa31462c8","output-custom-ctrl":[{"urn:uuid:4c960148-31ac-4de7-bada-918a99bdc29f":{"uuid":"urn:uuid:4c960148-31ac-4de7-bada-918a99bdc29f","deviceDefinitions":{"mac":"0000000000008000","downlinkType":"continuous","networkDefinitions":{"fport":10}},"active":1}}]}}
```
