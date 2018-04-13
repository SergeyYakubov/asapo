#!/usr/bin/env bash

set -e

# starts receiver on $service_node
# runs producer with various file sizes from $worker_node and measures performance

# a working directory
service_node=max-wgs
service_ip=`resolveip -s ${service_node}`
service_port=4201

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
scp ../../../cmake-build-release/examples/producer/dummy-data-producer/dummy-data-producer ${worker_node}:${worker_dir}


function do_work {
cat receiver.json |
  jq "to_entries |
       map(if .key == \"MonitorDbAddress\"
          then . + {value:\"${monitor_node}:${monitor_port}\"}
          elif .key == \"ListenPort\"
          then . + {value:${service_port}}
          elif .key == \"WriteToDisk\"
          then . + {value:$1}
          else .
          end
         ) |
      from_entries" > settings_tmp.json
scp settings_tmp.json ${service_node}:${service_dir}/settings.json
ssh ${service_node} "bash -c 'cd ${service_dir}; nohup ./receiver settings.json &> ${service_dir}/receiver.log &'"
sleep 0.3
for size  in 100 1000 10000
do
echo ===================================================================
ssh ${worker_node} ${worker_dir}/dummy-data-producer ${service_ip}:${service_port} ${size} 1000
ssh ${service_node} rm -f ${service_dir}/files/*
done
ssh ${service_node} killall receiver
}

echo
echo "With write to disk:"
do_work true

echo
echo "Without write to disk:"
do_work false


#rm settings_tmp.json
#ssh ${service_node} docker rm -f influxdb
