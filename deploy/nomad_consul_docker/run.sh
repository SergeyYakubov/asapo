#!/usr/bin/env bash

NOMAD_ALLOC_HOST_SHARED=/tmp/asapo/container_host_shared/nomad_alloc

ASAPO_USER=`id -u`:`id -g`

mkdir -p $NOMAD_ALLOC_HOST_SHARED
chmod 777 $NOMAD_ALLOC_HOST_SHARED

docker run --privileged --rm -v /var/run/docker.sock:/var/run/docker.sock \
  -u $ASAPO_USER \
  --group-add `getent group docker | cut -d: -f3` \
  -v /var/lib/docker:/var/lib/docker \
  -v $NOMAD_ALLOC_HOST_SHARED:$NOMAD_ALLOC_HOST_SHARED \
  -e NOMAD_ALLOC_DIR=$NOMAD_ALLOC_HOST_SHARED \
  --name asapo --net=host -d yakser/asapo-nomad-cluster

