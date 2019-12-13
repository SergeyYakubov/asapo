#!/usr/bin/env bash

NOMAD_ALLOC_HOST_SHARED=$HOME/asapo_client/container_host_shared/nomad_alloc
SERVICE_DATA_CLUSTER_SHARED=$HOME/asapo_client/cluster_shared/service_data
DATA_GLOBAL_SHARED=$HOME/asapo_client/global_shared/data

ASAPO_USER=`id -u`:`id -g`

NOMAD_TELEMETRY=true
NGINX_PORT_STREAM=8402
TELEGRAF_ADDRESS="127.0.0.1:${NGINX_PORT_STREAM}"

ASAPO_VAR_FILE=`pwd`/asapo_client_overwrite_vars.tfvars

mkdir -p $NOMAD_ALLOC_HOST_SHARED $SERVICE_DATA_CLUSTER_SHARED $DATA_GLOBAL_SHARED
chmod 777 $NOMAD_ALLOC_HOST_SHARED $SERVICE_DATA_CLUSTER_SHARED $DATA_GLOBAL_SHARED

cd $SERVICE_DATA_CLUSTER_SHARED
mkdir grafana influxdb
chmod 777 *

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
  -e TF_VAR_data_dir=$DATA_GLOBAL_SHARED \
  -e TF_VAR_nginx_port_stream=$NGINX_PORT_STREAM \
   $MOUNT_VAR_FILE \
  -e TF_VAR_asapo_user=$ASAPO_USER \
  -e TELEGRAF_ADDRESS=$TELEGRAF_ADDRESS \
  -e NOMAD_TELEMETRY=$NOMAD_TELEMETRY \
  --name asapo-client --net=host -d yakser/asapo-client-template

sleep 5