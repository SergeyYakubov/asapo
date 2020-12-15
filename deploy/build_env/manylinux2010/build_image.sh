#!/usr/bin/env bash

#docker build -t yakser/asapo-env:manylinux2010_ .
#./docker-squash yakser/asapo-env:manylinux2010_ -t yakser/asapo-env:manylinux2010

docker build -t yakser/asapo-env:manylinux2010 .
docker push yakser/asapo-env:manylinux2010



