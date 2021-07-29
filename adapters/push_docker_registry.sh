#!/bin/bash

sudo docker build -t localhost:50000/flint:lora-adapter -f LoraAgentDockerfile .
sudo docker push localhost:50000/flint:lora-adapter

sudo docker build -t localhost:50000/flint:sink -f SinkDockerfile .
sudo docker push localhost:50000/flint:sink

sudo docker build -t localhost:50000/flint:mapper -f MapperDockerfile .
sudo docker push localhost:50000/flint:mapper

sudo docker build -t localhost:50000/flint:udp-adapter -f UdpAgentDockerfile .
sudo docker push localhost:50000/flint:udp-adapter