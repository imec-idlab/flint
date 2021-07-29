#!/bin/bash

sudo docker build -t localhost:50000/bamoons/vno:lora-adapter -f LoraAgentDockerfile .
sudo docker push localhost:50000/bamoons/vno:lora-adapter

sudo docker build -t localhost:50000/bamoons/vno:sink -f SinkDockerfile .
sudo docker push localhost:50000/bamoons/vno:sink

sudo docker build -t localhost:50000/bamoons/vno:mapper -f MapperDockerfile .
sudo docker push localhost:50000/bamoons/vno:mapper

sudo docker build -t localhost:50000/bamoons/vno:udp-adapter -f UdpAgentDockerfile .
sudo docker push localhost:50000/bamoons/vno:udp-adapter