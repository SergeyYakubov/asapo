#!/usr/bin/env bash

# starts broker, mongodb on $service_node
# reads fileset into database
# calls getnext_broker example from $worker_node

nthreads=16
# a directory with many files in it
dir=/gpfs/petra3/scratch/yakubov/test
run_name=test
service_node=max-wgs

monitor_node=zitpcx27016
monitor_port=8086

worker_node=max-display
#worker_node=max-wgs

worker_dir=~/broker_test
service_dir=~/broker_test


cat settings.json |
  jq "to_entries |
       map(if .key == \"MonitorDbAddress\"
          then . + {value:\"${monitor_node}:${monitor_port}\"}
          else .
          end
         ) |
      from_entries" > settings_tmp.json

ssh ${monitor_node} influx -execute \"create database db_test\"

ssh ${service_node} docker run -d -p 27017:27017 --name mongo mongo
#ssh ${service_node} docker run -d -p 8086 -p 8086 --name influxdb influxdb

ssh ${service_node} mkdir ${service_dir}
ssh ${worker_node} mkdir ${worker_dir}


scp settings_tmp.json ${service_node}:${service_dir}/settings.json
rm settings_tmp.json
scp ../../../cmake-build-release/broker/hidra2-broker ${service_node}:${service_dir}
ssh ${service_node} "bash -c 'cd ${service_dir}; nohup ./hidra2-broker settings.json &> ${service_dir}/broker.log &'"
sleep 0.3
scp ../../../cmake-build-release/worker/tools/folder_to_db/folder2db ${worker_node}:${worker_dir}
ssh ${worker_node} ${worker_dir}/folder2db -n ${nthreads} ${dir} ${run_name} ${service_node}

sleep 3

scp ../../../cmake-build-release/examples/worker/getnext_broker/getnext_broker ${worker_node}:${worker_dir}
ssh ${worker_node} ${worker_dir}/getnext_broker ${service_node}:5005 ${run_name} ${nthreads}



ssh ${service_node} killall hidra2-broker
ssh ${service_node} docker rm -f mongo
#ssh ${service_node} docker rm -f influxdb
