#!/bin/bash
# Author Bart Moons, IDLab UGent.

# make sure to run as root
if [ "$EUID" -ne 0 ]
  then echo "ERROR: Please run as root"
  exit
fi

if [ ! ping -q -c 1 -W 1 8.8.8.8 >/dev/null ]; then
  echo "ERROR: Make sure you have internet access"
fi

# install docker
sudo apt-get update
sudo apt-get install -y apt-transport-https ca-certificates curl gnupg-agent software-properties-common

curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo apt-key add -

sudo apt-key fingerprint 0EBFCD88
sudo add-apt-repository "deb [arch=amd64] https://download.docker.com/linux/ubuntu $(lsb_release -cs) stable"

sudo apt-get update
sudo apt-get install docker-ce docker-ce-cli containerd.io

# configure firewall to allow pod-to-pod and pod-to-internet communication
sudo ufw allow in on cni0 && sudo ufw allow out on cni0

# run a local registry
sudo docker run -d -p 50000:50000 --name registry registry:2
