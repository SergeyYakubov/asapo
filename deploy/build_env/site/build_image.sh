#!/usr/bin/env bash

. ../../_docker_vars.sh

docker build -t $ASAPO_DOCKER_REPOSITORY/asapo-site-env -f Dockerfile .
if [ $ASAPO_DOCKER_DO_PUSH = "YES" ]; then
    docker push $ASAPO_DOCKER_REPOSITORY/asapo-site-env
fi
