#!/usr/bin/env bash

set -e

ASAPO_HOST_DIR=/var/tmp/asapo # you can change this if needed, make sure there is enough space ( >3GB on disk)
# change this according to your Docker configuration
DOCKER_ENDPOINT="127.0.0.1:2376"
DOCKER_TLS_CA=/usr/local/docker/certs/$USER/ca.pem
DOCKER_TLS_KEY=/usr/local/docker/certs/$USER/key.pem
DOCKER_TLS_CERT=/usr/local/docker/certs/$USER/cert.pem


NOMAD_ALLOC_HOST_SHARED=$ASAPO_HOST_DIR/container_host_shared/nomad_alloc
SERVICE_DATA_CLUSTER_SHARED=$ASAPO_HOST_DIR/asapo_cluster_shared/service_data
DATA_GLOBAL_SHARED=$ASAPO_HOST_DIR/global_shared/data
DATA_GLOBAL_SHARED_ONLINE=$ASAPO_HOST_DIR/global_shared/online_data
MONGO_DIR=$SERVICE_DATA_CLUSTER_SHARED/mongodb

ASAPO_USER=`id -u`:`id -g`

mkdir -p $NOMAD_ALLOC_HOST_SHARED $SERVICE_DATA_CLUSTER_SHARED $DATA_GLOBAL_SHARED $DATA_GLOBAL_SHARED_ONLINE
chmod 777 $NOMAD_ALLOC_HOST_SHARED $SERVICE_DATA_CLUSTER_SHARED $DATA_GLOBAL_SHARED $DATA_GLOBAL_SHARED_ONLINE

cd $SERVICE_DATA_CLUSTER_SHAREDdetector
mkdir -p fluentd grafana influxdb influxdb2 mongodb prometheus alertmanager
chmod 777 *

docker run --privileged --userns=host --security-opt no-new-privileges --rm \
  -u $ASAPO_USER \
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
  -v $DOCKER_TLS_CA:/etc/nomad/ca.pem \
  -v $DOCKER_TLS_KEY:/etc/nomad/key.pem \
  -v $DOCKER_TLS_CERT:/etc/nomad/cert.pem \
  -e DOCKER_ENDPOINT=$DOCKER_ENDPOINT \
  --name asapo --net=host -d yakser/asapo-cluster:22.03.0

sleep 15
docker exec asapo jobs-start
