#!/usr/bin/env bash
docker build -t yakser/asapo-env:centos7.9.2009 -f Dockerfile.7.9.2009 .
docker push yakser/asapo-env:centos7.9.2009