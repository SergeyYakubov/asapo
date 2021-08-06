#!/usr/bin/env bash

. ../_docker_vars.sh

sed "s/\$ASAPO_DOCKER_REPOSITORY/$ASAPO_DOCKER_REPOSITORY/" Dockerfile.template | docker build -t $ASAPO_DOCKER_REPOSITORY/asapo-orc . -f -

if [ $ASAPO_DOCKER_DO_PUSH = "YES" ]; then
    docker push $ASAPO_DOCKER_REPOSITORY/asapo-orc
fi
