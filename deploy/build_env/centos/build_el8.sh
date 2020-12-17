#!/usr/bin/env bash
docker build -t yakser/asapo-env:centos8.3.2011 -f Dockerfile.8.3.2011 .
docker push yakser/asapo-env:centos8.3.2011
