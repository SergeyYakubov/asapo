#!/usr/bin/env bash

set -e

trap Cleanup EXIT

producer_bin=$1
consumer_bin=$2
asapo_tool_bin=$3

beamtime_id=asapo_test

monitor_database_name=db_test
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
    influx -database ${monitor_database_name} -execute "drop series from statistics, RequestsRate"
}

token=`$asapo_tool_bin token -endpoint http://localhost:8400/asapo-authorizer -secret admin_token.key -types read $beamtime_id`


echo "Start producer"
mkdir -p ${receiver_folder}
$producer_bin localhost:8400 ${beamtime_id} 100 0 1 0 1000

echo "Start consumer in metadata only mode"
$consumer_bin ${proxy_address} ${receiver_folder} ${beamtime_id} 2 $token 1000 1 0 1 | tee out
grep "dummy_meta" out
