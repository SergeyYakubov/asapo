#!/usr/bin/env bash

set -e

. ../../_docker_vars.sh

docker build -t $ASAPO_DOCKER_REPOSITORY/asapo-env:centos7.9.2009 -f Dockerfile.7.9.2009 .

if [ $ASAPO_DOCKER_DO_PUSH = "YES" ]; then
    docker push $ASAPO_DOCKER_REPOSITORY/asapo-env:centos7.9.2009
fi
