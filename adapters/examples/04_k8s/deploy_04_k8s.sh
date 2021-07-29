# use this script to deploy all docker containers
# run with sudo ./deploy_vno.sh (--create-secrets) (--help)

if [[ "$@" =~ --help ]]; then
  echo 'Use this script to create secrets and/or deploy docker images from the container registry'
  echo '--create-secrets [create secrets to access private docker registries]'
  echo '--deploy [apply deplyoment files]'
  exit
fi

if [[ "$@" =~ --create-secrets ]]; then
	# create secrets to access private registries
fi

if [[ "$@" =~ --deploy ]]; then
	# deploy the mosquitto broker
	sudo microk8s kubectl apply -f ../../broker/mosquitto/mosquitto-deployment.yaml
	sudo microk8s kubectl apply -f ../../broker/mosquitto/mosquitto-service.yaml

	# deploy the mapper
	sudo microk8s kubectl apply -f mapper/deployment.yaml
	# deploy the lora adapter
	sudo microk8s kubectl apply -f lora/deployment.yaml
	# deploy the udp adapter
	sudo microk8s kubectl apply -f udp/deployment.yaml

fi

if [[ "$@" =~ --deploy-simulation ]]; then
	sudo microk8s kubectl apply -f ../adapters/agents/simulator
fi