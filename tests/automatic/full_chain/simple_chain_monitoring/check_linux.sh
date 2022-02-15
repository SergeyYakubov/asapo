#!/usr/bin/env bash

set -e

trap Cleanup EXIT

producer_bin=$1
consumer_bin=$2
asapo_tool_bin=$3

beamtime_id=asapo_test

monitor_database_name=db_test
new_monitor_database_name=asapo_monitoring

proxy_address=127.0.0.1:8400

beamline=test
receiver_root_folder=/tmp/asapo/receiver/files
facility=test_facility
year=2019
receiver_folder=${receiver_root_folder}/${facility}/gpfs/${beamline}/${year}/data/${beamtime_id}


Cleanup() {
    echo cleanup
    rm -rf ${receiver_root_folder}
    rm -rf out
    echo "db.dropDatabase()" | mongo ${beamtime_id}_detector
    set +e
    influx -database ${monitor_database_name} -execute "drop series from statistics, RequestsRate"
#    influx -database ${new_monitor_database_name} -execute "drop series from /.*/"
}

token=`$asapo_tool_bin token -endpoint http://localhost:8400/asapo-authorizer -secret admin_token.key -types read $beamtime_id`

sleep 5 # to write to influxdb old data and the clean up
influx -database ${new_monitor_database_name} -execute "drop series from /.*/"

echo "Start producer"
mkdir -p ${receiver_folder}
$producer_bin localhost:8400 ${beamtime_id} 100 100 4 0 100

echo "Start consumer in metadata only mode"
$consumer_bin ${proxy_address} ${receiver_folder} ${beamtime_id} 2 $token 2000 1 | tee out
grep "Processed 100 file(s)" out

#check monitoring
sleep 5 # to write to influxdb
influx -execute "select sum(requestedFileCount) from brokerRequests" -database=${new_monitor_database_name} -format=json | tee /dev/stderr  | jq .results[0].series[0].values[0][1] | tee /dev/stderr | grep 100
