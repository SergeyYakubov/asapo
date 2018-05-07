#!/usr/bin/env bash

set -e

trap Cleanup EXIT

#clean-up
Cleanup() {
set +e
ssh ${receiver_node} rm -f ${receiver_dir}/files/*
ssh ${receiver_node} killall receiver
ssh ${broker_node} killall asapo-broker
ssh ${broker_node} docker rm -f -v mongo
}

#monitoring_setup
monitor_node=zitpcx27016
monitor_port=8086


#logs
log_dir=~/fullchain_tests/logs

# starts receiver on $receiver_node
# runs producer with various file sizes from $producer_node and measures performance

file_size=1000
file_num=$((10000000 / $file_size))
echo filesize: ${file_size}K, filenum: $file_num

# receiver_setup
receiver_node=max-wgs
receiver_ip=`resolveip -s ${receiver_node}`
receiver_port=4201
receiver_dir=/gpfs/petra3/scratch/yakubov/receiver_tests
ssh ${receiver_node} mkdir -p ${receiver_dir}/logs
ssh ${receiver_node} mkdir -p ${receiver_dir}/files
scp ../../../cmake-build-release/receiver/receiver ${receiver_node}:${receiver_dir}
cat receiver.json |
  jq "to_entries |
       map(if .key == \"MonitorDbAddress\"
          then . + {value:\"${monitor_node}:${monitor_port}\"}
          elif .key == \"ListenPort\"
          then . + {value:${receiver_port}}
          else .
          end
         ) |
      from_entries" > settings_tmp.json
scp settings_tmp.json ${receiver_node}:${receiver_dir}/settings.json

#producer_setup
producer_node=max-display001
#producer_node=max-wgs
producer_dir=~/fullchain_tests
scp ../../../cmake-build-release/examples/producer/dummy-data-producer/dummy-data-producer ${producer_node}:${producer_dir}

#broker_setup
broker_node=max-wgs
broker_dir=~/fullchain_tests
ssh ${broker_node} mkdir -p ${broker_dir}/logs
cat broker.json |
  jq "to_entries |
       map(if .key == \"MonitorDbAddress\"
          then . + {value:\"${monitor_node}:${monitor_port}\"}
          else .
          end
         ) |
      from_entries" > settings_tmp.json
scp settings_tmp.json ${broker_node}:${broker_dir}/broker.json
rm settings_tmp.json
scp ../../../cmake-build-release/broker/asapo-broker ${broker_node}:${broker_dir}


#worker_setup
worker_node=max-display002
worker_dir=~/fullchain_tests
nthreads=16
scp ../../../cmake-build-release/examples/worker/getnext_broker/getnext_broker ${worker_node}:${worker_dir}

#monitoring_start
ssh ${monitor_node} influx -execute \"create database db_test\"
#ssh ${monitor_node} docker run -d -p 8086 -p 8086 --name influxdb influxdb

#mongo_start
ssh ${broker_node} docker run -d -p 27017:27017 --name mongo mongo

#receiver_start
ssh ${receiver_node} "bash -c 'cd ${receiver_dir}; nohup ./receiver settings.json &> ${log_dir}/log.receiver &'"
sleep 0.3

#broker_start
ssh ${broker_node} "bash -c 'cd ${broker_dir}; nohup ./asapo-broker -config broker.json &> ${log_dir}/log.broker &'"
sleep 0.3

#producer_start
ssh ${producer_node} "bash -c 'cd ${producer_dir}; nohup ./dummy-data-producer ${receiver_ip}:${receiver_port} ${file_size} ${file_num} &> ${producer_dir}/producer.log &'"
sleep 0.3

#worker_start
ssh ${worker_node} ${worker_dir}/getnext_broker ${broker_node}:5005 test_run ${nthreads}

