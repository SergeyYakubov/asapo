#!/usr/bin/env bash
set -e

vers="ubuntu16.04 ubuntu18.04 debian9.13 debian10.7"

for ver in $vers
do
    docker build -t yakser/asapo-env:${ver} -f Dockerfile_${ver} --build-arg OS=${ver} .
    docker push yakser/asapo-env:${ver}
done



