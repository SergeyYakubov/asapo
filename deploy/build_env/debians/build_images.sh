#!/usr/bin/env bash
set -e

. ../../_docker_vars.sh

vers="ubuntu18.04 ubuntu16.04 debian9.13 debian10.7 debian11.1"

for ver in $vers
do
    docker build -t $ASAPO_DOCKER_REPOSITORY/asapo-env:${ver} -f Dockerfile_${ver} --build-arg OS=${ver} .

    if [ $ASAPO_DOCKER_DO_PUSH = "YES" ]; then
        docker push $ASAPO_DOCKER_REPOSITORY/asapo-env:${ver}
    fi
done
