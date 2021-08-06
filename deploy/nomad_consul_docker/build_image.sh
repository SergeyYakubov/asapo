#!/usr/bin/env bash

set -e

. ../_docker_vars.sh

docker build -t $ASAPO_DOCKER_REPOSITORY/asapo-nomad-cluster .
if [ $ASAPO_DOCKER_DO_PUSH = "YES" ]; then
    docker push $ASAPO_DOCKER_REPOSITORY/asapo-nomad-cluster
fi
