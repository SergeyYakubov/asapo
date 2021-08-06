#!/usr/bin/env bash

set -e

. ../../_docker_vars.sh

docker build -t $ASAPO_DOCKER_REPOSITORY/asapo-env:centos8.3.2011 -f Dockerfile.8.3.2011 .

if [ $ASAPO_DOCKER_DO_PUSH = "YES" ]; then
    docker push $ASAPO_DOCKER_REPOSITORY/asapo-env:centos8.3.2011
fi
