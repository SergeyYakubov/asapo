#!/usr/bin/env bash

NOMAD_ALLOC_HOST_SHARED=/tmp/asapo/container_host_shared/nomad_alloc
SERVICE_DATA_CLUSTER_SHARED=/tmp/asapo/asapo_cluster_shared/service_data
DATA_GLOBAL_SHARED=/tmp/asapo/global_shared/data

mkdir -p $NOMAD_ALLOC_HOST_SHARED $SERVICE_DATA_CLUSTER_SHARED $DATA_GLOBAL_SHARED
chmod 777 $NOMAD_ALLOC_HOST_SHARED $SERVICE_DATA_CLUSTER_SHARED $DATA_GLOBAL_SHARED

docker run --privileged --rm -v /var/run/docker.sock:/var/run/docker.sock \
 	-v /var/lib/docker:/var/lib/docker \
	-v $NOMAD_ALLOC_HOST_SHARED:$NOMAD_ALLOC_HOST_SHARED \
	-v $SERVICE_DATA_CLUSTER_SHARED:$SERVICE_DATA_CLUSTER_SHARED \
	-v $DATA_GLOBAL_SHARED:$DATA_GLOBAL_SHARED \
	-e NOMAD_ALLOC=$NOMAD_ALLOC_HOST_SHARED \
	-e TF_VAR_service_dir=$SERVICE_DATA_CLUSTER_SHARED \
	-e TF_VAR_data_dir=$DATA_GLOBAL_SHARED \
 	--name asapo --net=host -d yakser/asapo-cluster

