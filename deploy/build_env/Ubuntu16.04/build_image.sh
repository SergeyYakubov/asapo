#!/usr/bin/env bash
set -e
docker build -t yakser/asapo-env:ubuntu16.04 .
docker push yakser/asapo-env:ubuntu16.04
