#!/usr/bin/env bash

set -e

. ../_docker_vars.sh

IMAGE=$ASAPO_DOCKER_REPOSITORY/asapo-cluster:20.06.3

#folders
NOMAD_ALLOC_HOST_SHARED=/var/tmp/asapo/container_host_shared/nomad_alloc
SERVICE_DATA_CLUSTER_SHARED=/home/yakubov/asapo/asapo_cluster_shared/service_data
DATA_GLOBAL_SHARED=/gpfs/petra3/scratch/yakubov/asapo_shared
DATA_GLOBAL_SHARED_ONLINE=/gpfs/petra3/scratch/yakubov/asapo_shared_online
MONGO_DIR=/scratch/mongodb # due to performance reasons mongodb can benefit from writing to local filesystem (HA to be worked on)
#service distribution
MAX_NOMAD_SERVERS=3 #  rest are clients
N_ASAPO_LIGHTWEIGHT_SERVICE_NODES=1 # where to put influx, elk, ... . Rest are receivers, brokers, mongodb

#DESY stuff
RECURSORS=["\"131.169.40.200\"",\""131.169.194.200\""]

ASAPO_USER=`id -u`:`id -g`

ASAPO_VAR_FILE=`pwd`/asapo_overwrite_vars.tfvars
ACL_ENABLED=true


#docker stuff
DOCKER_ENDPOINT="127.0.0.1:2376" #comment to use unix sockets
DOCKER_TLS_CA=/beegfs/desy/sys/docker/certs/ca.pem
DOCKER_TLS_KEY=/beegfs/desy/sys/docker/certs/$USER/key.pem
DOCKER_TLS_CERT=/beegfs/desy/sys/docker/certs/$USER/cert.pem

#adresses to use
USE_IB_FOR_RECEIVER=true
if [ "$USE_IB_FOR_RECEIVER" == "true" ]; then
IB_HOSTNAME=`hostname --short`-ib
IB_ADDRESS=`getent hosts $IB_HOSTNAME | awk '{ print $1 }'`
fi
#ADVERTISE_IP=  #set if differs from default

#prepare env variables based on the above input
N_SERVERS=$(( $SLURM_JOB_NUM_NODES > $MAX_NOMAD_SERVERS ? $MAX_NOMAD_SERVERS : $SLURM_JOB_NUM_NODES ))

SERVER_ADRESSES=`scontrol show hostnames $SLURM_JOB_NODELIST | head -$N_SERVERS | awk 'BEGIN{printf "["} {printf "%s\"%s\"",sep,$0; sep=","} END{print "]"}'`
ASAPO_LIGHTWEIGHT_SERVICE_NODES=`scontrol show hostnames $SLURM_JOB_NODELIST | head -$N_ASAPO_LIGHTWEIGHT_SERVICE_NODES | awk 'BEGIN{printf "["} {printf "%s\"%s\"",sep,$0; sep=","} END{print "]"}'`

# make folders if not exist
mkdir -p $NOMAD_ALLOC_HOST_SHARED $SERVICE_DATA_CLUSTER_SHARED $DATA_GLOBAL_SHARED $MONGO_DIR
chmod 777 $NOMAD_ALLOC_HOST_SHARED $SERVICE_DATA_CLUSTER_SHARED $DATA_GLOBAL_SHARED $MONGO_DIR
cd $SERVICE_DATA_CLUSTER_SHARED
mkdir esdatadir fluentd grafana influxdb mongodb prometheus alertmanager
chmod 777 *

#todo: elastic search check
mmc=`cat /proc/sys/vm/max_map_count`
if (( mmc < 262144 )); then
echo consider increasing max_map_count - needed for elasticsearch
#    exit 1
fi

docker rm -f asapo &>/dev/null

docker pull $IMAGE

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
-e TF_VAR_online_dir=$DATA_GLOBAL_SHARED_ONLINE \
-e TF_VAR_offline_dir=$DATA_GLOBAL_SHARED \
-e TF_VAR_mongo_dir=$MONGO_DIR \
-e TF_VAR_asapo_user=$ASAPO_USER \
-e ADVERTISE_IP=$ADVERTISE_IP \
-e RECURSORS=$RECURSORS \
-e IB_ADDRESS=$IB_ADDRESS \
-e ACL_ENABLED=$ACL_ENABLED \
-e SERVER_ADRESSES=$SERVER_ADRESSES \
-e ASAPO_LIGHTWEIGHT_SERVICE_NODES=$ASAPO_LIGHTWEIGHT_SERVICE_NODES \
-e DOCKER_ENDPOINT=$DOCKER_ENDPOINT \
-e N_SERVERS=$N_SERVERS \
--name asapo $IMAGE
