#!/usr/bin/env bash

set -e

trap Cleanup EXIT

#clean-up
Cleanup() {
set +e
ssh ${receiver_node} rm -f ${receiver_dir}/files/${beamline}/${beamtime_id}/*
ssh ${receiver_node} killall receiver
ssh ${receiver_node} killall asapo-discovery
ssh ${receiver_node} killall asapo-authorizer
ssh ${broker_node} killall asapo-broker
ssh ${broker_node} docker rm -f -v mongo
}

beamtime_id=asapo_test
beamline=test


#monitoring_setup
monitor_node=zitpcx27016
monitor_port=8086


#logs
log_dir=~/fullchain_tests/logs

# starts receiver on $receiver_node
# runs producer with various file sizes from $producer_node and measures performance

file_size=10000
file_num=$((10000000 / $file_size))
#file_num=$((100000 / $file_size))
echo filesize: ${file_size}K, filenum: $file_num

# receiver_setup
receiver_node=max-wgs
receiver_port=4201
receiver_dir=/gpfs/petra3/scratch/yakubov/receiver_tests
ssh ${receiver_node} mkdir -p ${receiver_dir}/files/${beamline}/${beamtime_id}
scp ../../../cmake-build-release/receiver/receiver ${receiver_node}:${receiver_dir}
cat receiver.json |
  jq "to_entries |
       map(if .key == \"PerformanceDbServer\"
          then . + {value:\"${monitor_node}:${monitor_port}\"}
          elif .key == \"ListenPort\"
          then . + {value:${receiver_port}}
          else .
          end
         ) |
      from_entries" > receiver_tmp.json

scp receiver_tmp.json ${receiver_node}:${receiver_dir}/receiver.json
rm receiver_tmp.json

#authorizer_setup
scp ../../../cmake-build-release/authorizer/asapo-authorizer ${receiver_node}:${receiver_dir}
scp authorizer.json ${receiver_node}:${receiver_dir}/authorizer.json


# discovery_setup
discovery_port=5006
cat discovery.json |
  jq "to_entries |
       map(if .key == \"Port\"
          then . + {value:${discovery_port}}
          elif .key == \"Endpoints\"
          then . + {value:[\"${receiver_node}:${receiver_port}\"]}
          else .
          end
         ) |
      from_entries" > discovery_tmp.json

cat discovery_tmp.json | jq ".Broker.StaticEndpoint = \"${receiver_node}:5005\"" > discovery_tmp1.json

cat discovery_tmp1.json | jq ".Receiver.StaticEndpoints = [\"${receiver_node}:${receiver_port}\"]" > discovery_tmp2.json

scp ../../../cmake-build-release/discovery/asapo-discovery ${receiver_node}:${receiver_dir}
scp discovery_tmp2.json ${receiver_node}:${receiver_dir}/discovery.json
discovery_ip=`resolveip -s ${receiver_node}`
rm discovery_tmp*.json

#producer_setup
producer_node=max-display001
#producer_node=max-wgs
producer_dir=~/fullchain_tests
scp ../../../cmake-build-release/examples/producer/dummy-data-producer/dummy-data-producer ${producer_node}:${producer_dir}
producer_nthreads=16


#broker_setup
broker_node=max-wgs
broker_dir=~/fullchain_tests
ssh ${broker_node} mkdir -p ${broker_dir}/logs
cat broker.json |
  jq "to_entries |
       map(if .key == \"PerformanceDbServer\"
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
scp ../../../cmake-build-release/asapo_tools/asapo ${worker_node}:${worker_dir}
scp ../../../tests/automatic/settings/auth_secret.key ${worker_node}:${worker_dir}/auth_secret.key

#monitoring_start
#ssh ${monitor_node} docker run -d -p 8086 -p 8086 --name influxdb influxdb

#mongo_start
ssh ${broker_node} docker run -d -p 27017:27017 --name mongo mongo

#discovery_start
ssh ${receiver_node} "bash -c 'cd ${receiver_dir}; nohup ./asapo-discovery -config discovery.json &> ${log_dir}/discovery.log &'"
sleep 0.3

#authorizer_start
ssh ${receiver_node} "bash -c 'cd ${receiver_dir}; nohup ./asapo-authorizer -config authorizer.json &> ${log_dir}/log.authorizer &'"

#receiver_start
ssh ${receiver_node} "bash -c 'cd ${receiver_dir}; nohup ./receiver receiver.json &> ${log_dir}/log.receiver &'"
sleep 0.3

#broker_start
ssh ${broker_node} "bash -c 'cd ${broker_dir}; nohup ./asapo-broker -config broker.json &> ${log_dir}/log.broker &'"
sleep 0.3

sleep 5

#producer_start
ssh ${producer_node} "bash -c 'cd ${producer_dir}; nohup ./dummy-data-producer ${receiver_node}:8400 ${beamtime_id} ${file_size} ${file_num} ${producer_nthreads} 0 100 &> ${log_dir}/producer.log &'"

sleep 1

#prepare token
ssh ${worker_node} "bash -c '${worker_dir}/asapo token -secret ${worker_dir}/auth_secret.key ${beamtime_id} >${worker_dir}/token'"
#worker_start
ssh ${worker_node} "bash -c '${worker_dir}/getnext_broker ${receiver_node}:8400 ${beamtime_id} ${nthreads} \`cat ${worker_dir}/token\`'"


