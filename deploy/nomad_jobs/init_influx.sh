#!/usr/bin/env bash

influx=`dig +short @127.0.0.1 influxdb.service.asapo | head -1`

databases="asapo_receivers asapo_brokers"

for database in $databases
do
    curl -i -XPOST http://${influx}:8086/query --data-urlencode "q=CREATE DATABASE $database"
done