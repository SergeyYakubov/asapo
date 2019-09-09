#!/usr/bin/env bash

docker exec -w /var/run/asapo asapo terraform init

docker exec -w /var/run/asapo asapo terraform apply -auto-approve -var fluentd_logs=false