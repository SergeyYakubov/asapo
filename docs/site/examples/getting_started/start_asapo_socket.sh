#!/usr/bin/env bash

set -e

ASAPO_HOST_DIR=/var/tmp/asapo # you can change this if needed, make sure there is enough space ( >3GB on disk)

NOMAD_ALLOC_HOST_SHARED=$ASAPO_HOST_DIR/container_host_shared/nomad_alloc
SERVICE_DATA_CLUSTER_SHARED=$ASAPO_HOST_DIR/asapo_cluster_shared/service_data
DATA_GLOBAL_SHARED=$ASAPO_HOST_DIR/global_shared/data
DATA_GLOBAL_SHARED_ONLINE=$ASAPO_HOST_DIR/global_shared/online_data
MONGO_DIR=$SERVICE_DATA_CLUSTER_SHARED/mongodb

ASAPO_USER=`id -u`:`id -g`

mkdir -p $NOMAD_ALLOC_HOST_SHARED $SERVICE_DATA_CLUSTER_SHARED $DATA_GLOBAL_SHARED $DATA_GLOBAL_SHARED_ONLINE
chmod 777 $NOMAD_ALLOC_HOST_SHARED $SERVICE_DATA_CLUSTER_SHARED $DATA_GLOBAL_SHARED $DATA_GLOBAL_SHARED_ONLINE

cd $SERVICE_DATA_CLUSTER_SHARED
mkdir -p fluentd grafana influxdb influxdb2 mongodb
chmod 777 *

docker run --privileged --rm -v /var/run/docker.sock:/var/run/docker.sock \
  -u $ASAPO_USER \
  --group-add `getent group docker | cut -d: -f3` \
  -v $NOMAD_ALLOC_HOST_SHARED:$NOMAD_ALLOC_HOST_SHARED \
  -v $SERVICE_DATA_CLUSTER_SHARED:$SERVICE_DATA_CLUSTER_SHARED \
  -v $DATA_GLOBAL_SHARED:$DATA_GLOBAL_SHARED \
  -e NOMAD_ALLOC_DIR=$NOMAD_ALLOC_HOST_SHARED \
  -e TF_VAR_service_dir=$SERVICE_DATA_CLUSTER_SHARED \
  -e TF_VAR_online_dir=$DATA_GLOBAL_SHARED_ONLINE \
  -e TF_VAR_offline_dir=$DATA_GLOBAL_SHARED \
  -e TF_VAR_mongo_dir=$MONGO_DIR \
  -e TF_VAR_asapo_user=$ASAPO_USER \
  -e ACL_ENABLED=true \
  --name asapo --net=host -d yakser/asapo-cluster:@ASAPO_VERSION_IN_DOCS@

sleep 15
docker exec asapo jobs-start -var elk_logs=false -var influxdb_version=1.8.4