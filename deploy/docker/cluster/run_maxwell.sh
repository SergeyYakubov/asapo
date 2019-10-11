#!/usr/bin/env bash

#folders
NOMAD_ALLOC_HOST_SHARED=/tmp/asapo/container_host_shared/nomad_alloc
SERVICE_DATA_CLUSTER_SHARED=/home/yakubov/asapo/asapo_cluster_shared/service_data
DATA_GLOBAL_SHARED=/home/yakubov/asapo/global_shared/data

#service distribution
MAX_NOMAD_SERVERS=3 #  rest are clients
N_ASAPO_LIGHTWEIGHT_SERVICE_NODES=1 # where to put influx, elk, ... . Rest are receivers, brokers, mongodb

#DESY stuff
RECURSORS=["\"131.169.40.200\"",\""131.169.194.200\""]

ASAPO_USER=`id -u`:`id -g`

ASAPO_VAR_FILE=`pwd`/asapo_overwrite_vars.tfvars


# use ib interface for service discovery (all communications goes thourgh this interface)
# todo: use ib only for communications with receiver (asapo discovery service should return correct ip using node meta IB_ADDRESS)
USE_IP_OVER_IB=true

#docker stuff
DOCKER_ENDPOINT="127.0.0.1:2376" #comment to use unix sockets
DOCKER_TLS_CA=/data/netapp/docker/certs/ca.pem
DOCKER_TLS_KEY=/data/netapp/docker/certs/$USER/key.pem
DOCKER_TLS_CERT=/data/netapp/docker/certs/$USER/cert.pem

IB_HOSTNAME=`hostname --short`-ib
IB_ADDRESS=`getent hosts $IB_HOSTNAME | awk '{ print $1 }'`

if [ "$USE_IP_OVER_IB" == "true" ]; then
  ADVERTISE_IP=$IB_ADDRESS
  HOSTNAME_SUFFIX=-ib
fi

#prepare env variables based on the above input
N_SERVERS=$(( $SLURM_JOB_NUM_NODES > $MAX_NOMAD_SERVERS ? $MAX_NOMAD_SERVERS : $SLURM_JOB_NUM_NODES ))

SERVER_ADRESSES=`scontrol show hostnames $SLURM_JOB_NODELIST | head -$N_SERVERS | awk -v suf=$HOSTNAME_SUFFIX 'BEGIN{printf "["} {printf "%s\"%s%s\"",sep,$0,suf; sep=","} END{print "]"}'`
ASAPO_LIGHTWEIGHT_SERVICE_NODES=`scontrol show hostnames $SLURM_JOB_NODELIST | head -$N_ASAPO_LIGHTWEIGHT_SERVICE_NODES | awk -v suf=$HOSTNAME_SUFFIX 'BEGIN{printf "["} {printf "%s\"%s%s\"",sep,$0,suf; sep=","} END{print "]"}'`

# make folders if not exist
mkdir -p $NOMAD_ALLOC_HOST_SHARED $SERVICE_DATA_CLUSTER_SHARED $DATA_GLOBAL_SHARED
chmod 777 $NOMAD_ALLOC_HOST_SHARED $SERVICE_DATA_CLUSTER_SHARED $DATA_GLOBAL_SHARED
cd $SERVICE_DATA_CLUSTER_SHARED
mkdir esdatadir fluentd grafana influxdb mongodb
chmod 777 *

#todo: elastic search check
mmc=`cat /proc/sys/vm/max_map_count`
if (( mmc < 262144 )); then
 	echo consider increasing max_map_count - needed for elasticsearch
#    exit 1
fi

docker rm -f asapo

docker pull yakser/asapo-cluster

if [ -f $ASAPO_VAR_FILE ]; then
  MOUNT_VAR_FILE="-v $ASAPO_VAR_FILE:/var/run/asapo/user_vars.tfvars"
fi

dockerrun --rm  \
	-u $ASAPO_USER \
 	-v /scratch/docker/100000.100000:/scratch/docker/100000.100000 \
	-v $NOMAD_ALLOC_HOST_SHARED:$NOMAD_ALLOC_HOST_SHARED \
	-v $SERVICE_DATA_CLUSTER_SHARED:$SERVICE_DATA_CLUSTER_SHARED \
	-v $DOCKER_TLS_CA:/etc/nomad/ca.pem \
	-v $DOCKER_TLS_KEY:/etc/nomad/key.pem \
	-v $DOCKER_TLS_CERT:/etc/nomad/cert.pem \
	-v $DATA_GLOBAL_SHARED:$DATA_GLOBAL_SHARED \
	 $MOUNT_VAR_FILE \
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

