#!/usr/bin/env bash

# starts broker, mongodb on $service_node
# reads fileset into database
# calls getnext_broker example from $worker_node

nthreads=1
# a directory with many files in it
dir=/gpfs/petra3/scratch/yakubov/test
run_name=test_run
token=K38Mqc90iRv8fC7prcFHd994mF_wfUiJnWBfIjIzieo=

service_node=max-wgs

monitor_node=zitpcx27016
monitor_port=8086

worker_node=max-display
#worker_node=max-wgs

worker_dir=~/broker_test
service_dir=~/broker_test


cat settings.json | jq ".PerformanceDbServer = \"${monitor_node}:${monitor_port}\"" > settings_tmp.json

cat discovery.json | jq ".Broker.StaticEndpoint = \"${service_node}:5005\"" > discovery_tmp.json


ssh ${monitor_node} influx -execute \"create database db_test\"

ssh ${service_node} docker run -d -p 27017:27017 --name mongo mongo
#ssh ${service_node} docker run -d -p 8086 -p 8086 --name influxdb influxdb

ssh ${service_node} mkdir -p ${service_dir}
ssh ${worker_node} mkdir -p ${worker_dir}


scp ../../../cmake-build-release/discovery/asapo-discovery ${service_node}:${service_dir}
scp discovery_tmp.json ${service_node}:${service_dir}/discovery.json
rm discovery_tmp.json

ssh ${service_node} "bash -c 'cd ${service_dir}; nohup ./asapo-discovery -config discovery.json &> ${service_dir}/discovery.log &'"


scp settings_tmp.json ${service_node}:${service_dir}/settings.json

scp ../../../tests/automatic/settings/auth_secret.key ${service_node}:${service_dir}/auth_secret.key



rm settings_tmp.json
scp ../../../cmake-build-release/broker/asapo-broker ${service_node}:${service_dir}
ssh ${service_node} "bash -c 'cd ${service_dir}; nohup ./asapo-broker -config settings.json &> ${service_dir}/broker.log &'"
sleep 0.3
scp ../../../cmake-build-release/worker/tools/folder_to_db/folder2db ${worker_node}:${worker_dir}
ssh ${worker_node} ${worker_dir}/folder2db -n ${nthreads} ${dir} ${run_name} ${service_node}

sleep 3

scp ../../../cmake-build-release/examples/worker/getnext_broker/getnext_broker ${worker_node}:${worker_dir}
ssh ${worker_node} ${worker_dir}/getnext_broker ${service_node}:8400 ${run_name} ${nthreads} $token



ssh ${service_node} killall asapo-broker
ssh ${service_node} killall asapo-discovery

ssh ${service_node} docker rm -f mongo
#ssh ${service_node} docker rm -f influxdb
