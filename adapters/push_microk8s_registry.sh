#!/bin/bash

sudo docker build -t localhost:32000/flint:lora-adapter -f LoraAgentDockerfile .
sudo docker push localhost:32000/flint:lora-adapter

sudo docker build -t localhost:32000/flint:sink -f SinkDockerfile .
sudo docker push localhost:32000/flint:sink

sudo docker build -t localhost:32000/flint:mapper -f MapperDockerfile .
sudo docker push localhost:32000/flint:mapper

sudo docker build -t localhost:32000/flint:udp-adapter -f UdpAgentDockerfile .
sudo docker push localhost:32000/flint:udp-adapter