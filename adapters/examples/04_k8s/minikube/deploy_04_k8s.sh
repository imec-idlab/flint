# use this script to deploy all docker containers
# run with sudo ./deploy_vno.sh (--create-secrets) (--help)

if [[ "$@" =~ --help ]]; then
  echo 'Use this script to create secrets and/or deploy docker images from the container registry'
  echo '--create-secrets [create secrets to access private docker registries]'
  echo '--deploy-broker [apply broker deployment files]'
  echo '--deploy-adapters [apply deplyoment files]'
  exit
fi

if [[ "$@" =~ --deploy-broker ]]; then
	# deploy the mosquitto broker
	sudo microk8s kubectl apply -f ../../../broker/mosquitto/mosquitto-deployment.yaml
	sudo microk8s kubectl apply -f ../../../broker/mosquitto/mosquitto-service.yaml
fi

if [[ "$@" =~ --deploy-adapters ]]; then
	# deploy the mapper
	sudo microk8s kubectl apply -f mapper/deployment.yaml
	# deploy the lora adapter
	sudo microk8s kubectl apply -f lora/deployment.yaml
	# deploy the udp adapter
	sudo microk8s kubectl apply -f udp/deployment.yaml

fi