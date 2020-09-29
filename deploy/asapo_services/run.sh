#!/usr/bin/env bash

NOMAD_ALLOC_HOST_SHARED=/var/tmp/asapo/container_host_shared/nomad_alloc
SERVICE_DATA_CLUSTER_SHARED=/var/tmp/asapo/asapo_cluster_shared/service_data
DATA_GLOBAL_SHARED=/var/tmp/asapo/global_shared/data
DATA_GLOBAL_SHARED_ONLINE=/var/tmp/asapo/global_shared/online_data

MONGO_DIR=$SERVICE_DATA_CLUSTER_SHARED/mongodb

ASAPO_USER=`id -u`:`id -g`

#ADVERTISE_IP=
#RECURSORS=
#IB_ADDRESS=
#SERVER_ADRESSES=
#N_SERVERS=

ASAPO_VAR_FILE=`pwd`/asapo_overwrite_vars.tfvars
ACL_ENABLED=true

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

if [ -f $ASAPO_VAR_FILE ]; then
chmod 666 $ASAPO_VAR_FILE
MOUNT_VAR_FILE="-v $ASAPO_VAR_FILE:/var/run/asapo/user_vars.tfvars"
fi


docker run --privileged --rm -v /var/run/docker.sock:/var/run/docker.sock \
-u $ASAPO_USER \
--group-add `getent group docker | cut -d: -f3` \
-v /var/lib/docker:/var/lib/docker \
-v $NOMAD_ALLOC_HOST_SHARED:$NOMAD_ALLOC_HOST_SHARED \
-v $SERVICE_DATA_CLUSTER_SHARED:$SERVICE_DATA_CLUSTER_SHARED \
-v $DATA_GLOBAL_SHARED:$DATA_GLOBAL_SHARED \
-e NOMAD_ALLOC_DIR=$NOMAD_ALLOC_HOST_SHARED \
-e TF_VAR_service_dir=$SERVICE_DATA_CLUSTER_SHARED \
-e TF_VAR_online_dir=$DATA_GLOBAL_SHARED_ONLINE \
-e TF_VAR_offline_dir=$DATA_GLOBAL_SHARED \
-e TF_VAR_mongo_dir=$MONGO_DIR \
 $MOUNT_VAR_FILE \
-e ADVERTISE_IP=$ADVERTISE_IP \
-e RECURSORS=$RECURSORS \
-e TF_VAR_asapo_user=$ASAPO_USER \
-e IB_ADDRESS=$IB_ADDRESS \
-e ACL_ENABLED=$ACL_ENABLED \
-e SERVER_ADRESSES=$SERVER_ADRESSES \
-e N_SERVERS=$N_SERVERS \
--name asapo --net=host -d yakser/asapo-cluster

