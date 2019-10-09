#!/usr/bin/env bash

NOMAD_ALLOC_HOST_SHARED=/tmp/asapo/container_host_shared/nomad_alloc
SERVICE_DATA_CLUSTER_SHARED=/home/yakubov/asapo/asapo_cluster_shared/service_data
DATA_GLOBAL_SHARED=/home/yakubov/asapo/global_shared/data
MAX_NOMAD_SERVERS=3 #  rest are clients
N_ASAPO_LIGHTWEIGHT_SERVICE_NODES=1 # where to put influx, elk, ... . Rest are receivers, brokers, mongodb
ASAPO_USER=`id -u`:`id -g`

USE_IB=True

DOCKER_ENDPOINT="127.0.0.1:2376" #comment to use unix sockets
DOCKER_TLS_CA=/data/netapp/docker/certs/ca.pem
DOCKER_TLS_KEY=/data/netapp/docker/certs/$USER/key.pem
DOCKER_TLS_CERT=/data/netapp/docker/certs/$USER/cert.pem

#ADVERTISE_IP=
IB_ADDRESS=`hostname --short`-ib

RECURSORS=["\"131.169.40.200\"",\""131.169.194.200\""]
N_SERVERS=$(( $SLURM_JOB_NUM_NODES > $MAX_NOMAD_SERVERS ? $MAX_NOMAD_SERVERS : $SLURM_JOB_NUM_NODES ))
SERVER_ADRESSES=`scontrol show hostnames $SLURM_JOB_NODELIST | head -$N_SERVERS | awk 'BEGIN{printf "["} {printf "%s\"%s\"",sep,$0; sep=","} END{print "]"}'`
ASAPO_LIGHTWEIGHT_SERVICE_NODES=`scontrol show hostnames $SLURM_JOB_NODELIST | head -$N_ASAPO_LIGHTWEIGHT_SERVICE_NODES | awk 'BEGIN{printf "["} {printf "%s\"%s\"",sep,$0; sep=","} END{print "]"}'`

mkdir -p $NOMAD_ALLOC_HOST_SHARED $SERVICE_DATA_CLUSTER_SHARED $DATA_GLOBAL_SHARED
chmod 777 $NOMAD_ALLOC_HOST_SHARED $SERVICE_DATA_CLUSTER_SHARED $DATA_GLOBAL_SHARED

cd $SERVICE_DATA_CLUSTER_SHARED
mkdir esdatadir fluentd grafana influxdb mongodb
chmod 777 *

mmc=`cat /proc/sys/vm/max_map_count`
if (( mmc < 262144 )); then
 	echo consider increasing max_map_count - needed for elasticsearch
#    exit 1
fi

docker rm -f asapo

docker pull yakser/asapo-cluster


dockerrun --rm  \
	-u $ASAPO_USER \
 	-v /scratch/docker/100000.100000:/scratch/docker/100000.100000 \
	-v $NOMAD_ALLOC_HOST_SHARED:$NOMAD_ALLOC_HOST_SHARED \
	-v $SERVICE_DATA_CLUSTER_SHARED:$SERVICE_DATA_CLUSTER_SHARED \
	-v $DOCKER_TLS_CA:/etc/nomad/ca.pem \
	-v $DOCKER_TLS_KEY:/etc/nomad/key.pem \
	-v $DOCKER_TLS_CERT:/etc/nomad/cert.pem \
	-v $DATA_GLOBAL_SHARED:$DATA_GLOBAL_SHARED \
	-e NOMAD_ALLOC_DIR=$NOMAD_ALLOC_HOST_SHARED \
	-e TF_VAR_service_dir=$SERVICE_DATA_CLUSTER_SHARED \
	-e TF_VAR_data_dir=$DATA_GLOBAL_SHARED \
	-e ADVERTISE_IP=$ADVERTISE_IP \
	-e RECURSORS=$RECURSORS \
	-e TF_VAR_asapo_user=$ASAPO_USER \
	-e IB_ADDRESS=$IB_ADDRESS \
	-e SERVER_ADRESSES=$SERVER_ADRESSES \
	-e ASAPO_LIGHTWEIGHT_SERVICE_NODES=$ASAPO_LIGHTWEIGHT_SERVICE_NODES \
	-e DOCKER_ENDPOINT=$DOCKER_ENDPOINT \
	-e N_SERVERS=$N_SERVERS \
 	--name asapo yakser/asapo-cluster

