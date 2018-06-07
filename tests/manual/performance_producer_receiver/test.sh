#!/usr/bin/env bash

set -e

trap Cleanup EXIT

Cleanup() {
set +e
ssh ${service_node} rm -f ${service_dir}/files/*
ssh ${service_node} killall receiver
ssh ${service_node} killall asapo-discovery
ssh ${service_node} docker rm -f -v mongo
}


# starts receiver on $service_node
# runs producer with various file sizes from $worker_node and measures performance

# a working directory
service_node=max-wgs
service_ip=`resolveip -s ${service_node}`
discovery_port=5006
receiver_port=4201

monitor_node=zitpcx27016
monitor_port=8086

worker_node=max-display
#worker_node=max-wgs

worker_dir=~/producer_tests
service_dir=/gpfs/petra3/scratch/yakubov/receiver_tests

ssh ${monitor_node} influx -execute \"create database db_test\"

#ssh ${monitor_node} docker run -d -p 8086 -p 8086 --name influxdb influxdb

ssh ${service_node} mkdir -p ${service_dir}
ssh ${service_node} mkdir -p ${service_dir}/files
ssh ${worker_node} mkdir -p ${worker_dir}

scp ../../../cmake-build-release/receiver/receiver ${service_node}:${service_dir}
scp ../../../cmake-build-release/discovery/asapo-discovery ${service_node}:${service_dir}
scp ../../../cmake-build-release/examples/producer/dummy-data-producer/dummy-data-producer ${worker_node}:${worker_dir}

function do_work {
cat receiver.json |
  jq "to_entries |
       map(if .key == \"MonitorDbAddress\"
          then . + {value:\"${monitor_node}:${monitor_port}\"}
          elif .key == \"ListenPort\"
          then . + {value:${receiver_port}}
          elif .key == \"WriteToDisk\"
          then . + {value:$1}
          else .
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
ssh ${service_node} "bash -c 'cd ${service_dir}; nohup ./receiver receiver.json &> ${service_dir}/receiver.log &'"
ssh ${service_node} "bash -c 'cd ${service_dir}; nohup ./asapo-discovery -config discovery.json &> ${service_dir}/discovery.log &'"

sleep 0.3
for size  in 100 1000 10000
do
ssh ${service_node} docker run -d -p 27017:27017 --name mongo mongo
echo ===================================================================
ssh ${worker_node} ${worker_dir}/dummy-data-producer ${service_ip}:8400 ${size} 1000 8 0
if [ "$1" == "true" ]
then
    ssh ${service_node} rm -f ${service_dir}/files/*
fi
ssh ${service_node} docker rm -f -v mongo
done
ssh ${service_node} killall receiver
ssh ${service_node} killall asapo-discovery
}

echo
echo "With write to disk:"
do_work true

echo
echo "Without write to disk:"
do_work false


#rm settings_tmp.json
#ssh ${service_node} docker rm -f influxdb

