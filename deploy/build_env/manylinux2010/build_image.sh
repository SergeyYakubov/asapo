#!/usr/bin/env bash
docker build -t yakser/asapo-env:manylinux2010 .
docker push yakser/asapo-env:manylinux2010
