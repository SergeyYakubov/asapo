#!/usr/bin/env bash

influx=`dig +short @127.0.0.1  influxdb.service.asapo | head -1`

curl -i -XPOST http://${influx}:8086/query --data-urlencode "q=CREATE DATABASE asapo_receivers"