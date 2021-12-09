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


mkdir -p /tmp/asapo/test_in/processed

Cleanup() {
    echo cleanup
    kill -9 $producerid
    rm -rf /tmp/asapo/test_in
    rm -rf ${receiver_folder}
    echo "db.dropDatabase()" | mongo ${beamtime_id}_detector
    rm out.txt

    set +e
    influx_out=`influx -database ${monitor_database_name} -execute "drop series from statistics, RequestsRate"`
    influx_status=$?
    echo "Influx output: ${influx_out}"
    if [ $influx_status -ne 0 ]; then
        if [[ $influx_out == *"401"* ]]; then
            echo "Ignoring auth error from influxdb"
        else
            echo "influxdb failed to delete test database '${monitor_database_name}'"
            exit $influx_status
        fi
    fi
}

token=`$asapo_tool_bin token -endpoint http://localhost:8400/asapo-authorizer -secret admin_token.key -types read $beamtime_id`


echo "Start producer"
mkdir -p ${receiver_folder}
$producer_bin test.json &
producerid=`echo $!`

sleep 1
mkdir  /tmp/asapo/test_in/processed/test1
mkdir  /tmp/asapo/test_in/processed/test2
sleep 1

echo -n hello1 > /tmp/asapo/test_in/processed/test1/file1
echo -n hello2 > /tmp/asapo/test_in/processed/test1/file2
echo -n hello3 > /tmp/asapo/test_in/processed/test2/file1

echo "Start consumer in $network_type mode"
$consumer_bin ${proxy_address} ${receiver_folder} ${beamtime_id} 2 $token 1000 0 | tee out.txt
grep "Processed 3 file(s)" out.txt
grep "hello1" out.txt
grep "hello2" out.txt
grep "hello3" out.txt

