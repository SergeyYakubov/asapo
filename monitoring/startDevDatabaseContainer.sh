#!/bin/bash

docker run -d -p 8086:8086 \
      --name asapo-monitoring-influxdb \
      -v $HOME/asapoInfluxdbData/data:/var/lib/influxdb2 \
      -v $HOME/asapoInfluxdbData/config:/etc/influxdb2 \
      -e DOCKER_INFLUXDB_INIT_MODE=setup \
      -e DOCKER_INFLUXDB_INIT_USERNAME=user \
      -e DOCKER_INFLUXDB_INIT_PASSWORD=password \
      -e DOCKER_INFLUXDB_INIT_ORG=asapo \
      -e DOCKER_INFLUXDB_INIT_BUCKET=asapo-monitoring \
      influxdb:2.0.7
