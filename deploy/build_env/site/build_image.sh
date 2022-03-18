#!/usr/bin/env bash
docker build -t yakser/asapo-site-env -f Dockerfile .
docker push yakser/asapo-site-env