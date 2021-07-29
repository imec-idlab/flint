# 04_k8s
This example deploys Docker containers in a Kubernetes cluster
1. LoRa adapter - MQTT client listening for packets on a broker (port 8000) and forwarding these to the UDP adapter
2. mosquitto - MQTT broker that serves as message bus
3. Mapper adapter - Mapper instance forwarding traffic between flint components
4. UDP adapter - UDP client forwarding traffic to a UDP server


## Run the example
### Set up k8s
Simply run the provided bootscript to install `Docker` and `microk8s`.
Start `microk8s`:
```
sudo microk8s start
sudo microk8s enable dashboard dns registry
```

Forward the dashboard:
```
sudo microk8s kubectl port-forward -n kube-system service/kubernetes-dashboard 10443:443
```

in order to access https, use the default token retrieved with:
```
token=$(sudo microk8s kubectl -n kube-system get secret | grep default-token | cut -d " " -f1)
sudo microk8s kubectl -n kube-system describe secret $token
```
... now accesbible using firefox at https://localhost:10443)

### Deploy containers
#### Push the containers to the microk8s registry
Ideally, you would want to use an external registry. However, local deployments can use the built in registry from `microk8s`. This can be enabled using `sudo microk8s enable registry`. In the root directory run the `push_microk8s_registry` script.
```
sudo bash ./push_microk8s_registry.sh
```

#### Configure the mosquitto broker
Change the `hostPath` of the configuration files for every `deployment.yaml` file:
```
volumes:
      - name: conf-lora
        hostPath:
          path: /home/../flint/broker/mosquitto/mosquitto-1883.conf

```
If required, you can make the broker accesible using the `externalIPs` field in the `mosquitto-service.yaml` file:
```
  externalIPs:
  - 0.0.0.0
```

#### Configure the adapters
Change the `hostPath` of the configuration files for every `deployment.yaml` file:
```
volumes:
      - name: conf-lora
        hostPath:
          path: /home/../flint/adapters/agents/lora/conf/

```
In order for the LoRa adapter to communicate with the local broker (port 8000), `hostNetwork` is set to `true`;


#### Deploy flint
First deploy the broker. Inside the root folder of this example, run the `deploy_04_k8s.sh` script.
```
sudo bash ./deploy_04_k8s.sh --deploy-broker
```
Now take note of the broker IP with
```
sudo microk8s kubectl get all --all-namespaces

> default              service/mosquitto-service           LoadBalancer   10.152.183.51    192.186.0.1   8001:32758/TCP           18m
```
Configure the `[sink]` section in every configuration file.
```
[sink]
id=urn:uuid:4c960148-31ac-4de7-bada-918a99bdc29f
broker-ip=10.152.183.51
broker-port=8001
```

Once configured, the adapters can be deployed with:
```
sudo bash ./deploy_04_k8s.sh --deploy-adapters
```

#### Send data
Once the complete system is up and running, you can open a `nc` window:

```
nc -u -l 5000
```

And send MQTT traffic using `mosquitto`:
```
mosquitto_pub -h 127.0.0.1 -p 8000 -t "application/1/device/8000/event/up" -m '{"devEUI": "8000", "data": "00040x1.83fb035237a27p+30", "fPort": 10, "txInfo": {"dr": 5, "frequency": 868300000}, "rxInfo": [{"gatewayID": "1dee0be9d4472aaf", "uplinkID": "270bb8f5-ccb0-4382-abb6-11e2a9774690", "name": "LoRank8", "rssi": -67, "loRaSNR": 10}]}'
```

And you should see the following output in your `nc`:
```
{"device-ctrl":{"data":"00040x1.83fb035237a27p+30","dev-eui":"0000000000008000","fPort":10,"device-custom-ctrl":{"txInfo":{"dr":5,"frequency":868300000},"rxInfo":[{"gatewayID":"1dee0be9d4472aaf","uplinkID":"270bb8f5-ccb0-4382-abb6-11e2a9774690","name":"LoRank8","rssi":-67,"loRaSNR":10}]}},"input-ctrl":{"input-type":"LoraAdapterAgent","input-ip":"127.0.0.1","input-port":8000,"input-protocol":"mqtt"},"adapter-ctrl":{"from":"urn:uuid:4c960148-31ac-4de7-bada-918a99bdc29f","adapter-scheme":[{"urn:uuid:4c960148-31ac-4de7-bada-918a99bdc29f":"urn:uuid:e4370d7c-eae3-416d-85f0-751d396cacf5"},{"urn:uuid:e4370d7c-eae3-416d-85f0-751d396cacf5":"urn:uuid:f927f1bd-477e-4f99-bc3b-7734d81dd863"}]},"output-ctrl":{"ipv6":"0:0:0:0:0:0:0:0","thing-id":"urn:uuid:9b70666c-ebae-428a-a13a-6fdaa31462c8","output-custom-ctrl":[{"urn:uuid:4c960148-31ac-4de7-bada-918a99bdc29f":{"uuid":"urn:uuid:4c960148-31ac-4de7-bada-918a99bdc29f","deviceDefinitions":{"mac":"0000000000008000","downlinkType":"continuous","networkDefinitions":{"fport":10}},"active":1}}]}}
```