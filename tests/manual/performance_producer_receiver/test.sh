#!/usr/bin/env bash

set -e

trap Cleanup EXIT

Cleanup() {
set +e
ssh ${service_node} rm -f ${service_dir}/files/${beamline}/${beamtime_id}/*
ssh ${service_node} killall receiver
ssh ${service_node} killall asapo-discovery
ssh ${service_node} killall asapo-authorizer
ssh ${service_node} docker rm -f -v mongo
}


# starts receiver on $service_node
# runs producer with various file sizes from $consumer_node and measures performance

# a working directory
service_node=max-wgs
service_ip=`resolveip -s ${service_node}`
discovery_port=5006
receiver_port=4201
beamtime_id=asapo_test
beamline=test


monitor_node=zitpcx27016
monitor_port=8086

consumer_node=max-display
#consumer_node=max-wgs

consumer_dir=~/producer_tests
service_dir=/gpfs/petra3/scratch/yakubov/receiver_tests

ssh ${monitor_node} influx -execute \"create database db_test\"

#ssh ${monitor_node} docker run -d -p 8086 -p 8086 --name influxdb influxdb

ssh ${service_node} mkdir -p ${service_dir}
ssh ${service_node} mkdir -p ${service_dir}/files/${beamtime_id}
ssh ${consumer_node} mkdir -p ${consumer_dir}

scp ../../../cmake-build-release/receiver/receiver ${service_node}:${service_dir}
scp ../../../cmake-build-release/discovery/asapo-discovery ${service_node}:${service_dir}

scp ../../../cmake-build-release/authorizer/asapo-authorizer ${service_node}:${service_dir}
scp authorizer.json ${service_node}:${service_dir}/authorizer.json

scp ../../../cmake-build-release/examples/producer/dummy-data-producer/dummy-data-producer ${consumer_node}:${consumer_dir}

function do_work {
cat receiver.json |
  jq "to_entries |
       map(if .key == \"PerformanceDbServer\"
          then . + {value:\"${monitor_node}:${monitor_port}\"}
          elif .key == \"ListenPort\"
          then . + {value:${receiver_port}}
          end
         ) |
      from_entries" > receiver_tmp.json

cat discovery.json |
  jq "to_entries |
       map(if .key == \"Port\"
          then . + {value:${discovery_port}}
          else .
          end
         ) |
      from_entries" > discovery_tmp.json

cat discovery.json | jq ".Port = ${discovery_port}" > discovery_tmp.json

cat discovery_tmp.json | jq ".Receiver.StaticEndpoints = [\"${service_node}:${receiver_port}\"]" > discovery_tmp1.json


scp discovery_tmp1.json ${service_node}:${service_dir}/discovery.json
scp receiver_tmp.json ${service_node}:${service_dir}/receiver.json


rm discovery_tmp*.json receiver_tmp.json
ssh ${service_node} "bash -c 'cd ${service_dir}; nohup ./asapo-authorizer -config authorizer.json &> ${service_dir}/authorizer.log &'"
ssh ${service_node} "bash -c 'cd ${service_dir}; nohup ./receiver receiver.json &> ${service_dir}/receiver.log &'"
ssh ${service_node} "bash -c 'cd ${service_dir}; nohup ./asapo-discovery -config discovery.json &> ${service_dir}/discovery.log &'"

sleep 0.3
for size  in 100 1000 10000
do
ssh ${service_node} docker run -d -p 27017:27017 --name mongo mongo
echo ===================================================================
ssh ${consumer_node} ${consumer_dir}/dummy-data-producer ${service_ip}:8400 ${beamtime_id} ${size} 10000 8 0 100
if [ "$1" == "true" ]
then
    ssh ${service_node} rm -f ${service_dir}/files/${beamline}/${beamtime_id}/*
fi
ssh ${service_node} docker rm -f -v mongo
done
ssh ${service_node} killall receiver
ssh ${service_node} killall asapo-discovery
ssh ${service_node} killall asapo-authorizer
}

echo
echo "With write to disk:"
do_work true

echo
echo "Without write to disk:"
do_work false


#rm settings_tmp.json
#ssh ${service_node} docker rm -f influxdb

