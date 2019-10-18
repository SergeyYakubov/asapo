#!/usr/bin/env bash

NOMAD_ALLOC_HOST_SHARED=/tmp/asapo/container_host_shared/nomad_alloc
SERVICE_DATA_CLUSTER_SHARED=/tmp/asapo/asapo_cluster_shared/service_data
DATA_GLOBAL_SHARED=/tmp/asapo/global_shared/data
MONGO_DIR=$SERVICE_DATA_CLUSTER_SHARED/mongodb

ASAPO_USER=`id -u`:`id -g`

#ADVERTISE_IP=
#RECURSORS=
#IB_ADDRESS=
#SERVER_ADRESSES=
#N_SERVERS=

mkdir -p $NOMAD_ALLOC_HOST_SHARED $SERVICE_DATA_CLUSTER_SHARED $DATA_GLOBAL_SHARED
chmod 777 $NOMAD_ALLOC_HOST_SHARED $SERVICE_DATA_CLUSTER_SHARED $DATA_GLOBAL_SHARED

cd $SERVICE_DATA_CLUSTER_SHARED
mkdir esdatadir fluentd grafana influxdb mongodb
chmod 777 *

mmc=`cat /proc/sys/vm/max_map_count`

if (( mmc < 262144 )); then
 	echo increase max_map_count - needed for elasticsearch
    exit 1
fi

docker run --privileged --rm -v /var/run/docker.sock:/var/run/docker.sock \
	-u $ASAPO_USER \
 	-v /var/lib/docker:/var/lib/docker \
	-v $NOMAD_ALLOC_HOST_SHARED:$NOMAD_ALLOC_HOST_SHARED \
	-v $SERVICE_DATA_CLUSTER_SHARED:$SERVICE_DATA_CLUSTER_SHARED \
	-v $DATA_GLOBAL_SHARED:$DATA_GLOBAL_SHARED \
	-e NOMAD_ALLOC_DIR=$NOMAD_ALLOC_HOST_SHARED \
	-e TF_VAR_service_dir=$SERVICE_DATA_CLUSTER_SHARED \
	-e TF_VAR_data_dir=$DATA_GLOBAL_SHARED \
	-e TF_VAR_mongo_dir=$MONGO_DIR \
	-e ADVERTISE_IP=$ADVERTISE_IP \
	-e RECURSORS=$RECURSORS \
	-e TF_VAR_asapo_user=$ASAPO_USER \
	-e IB_ADDRESS=$IB_ADDRESS \
	-e SERVER_ADRESSES=$SERVER_ADRESSES \
	-e N_SERVERS=$N_SERVERS \
 	--name asapo --net=host -d yakser/asapo-cluster

