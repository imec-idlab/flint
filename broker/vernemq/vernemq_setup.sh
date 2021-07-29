sudo microk8s enable helm3

sudo microk8s helm3 repo add vernemq https://vernemq.github.io/docker-vernemq
sudo microk8s helm3 repo update
sudo microk8s helm3 install -f values.yaml vernemq vernemq/vernemq

