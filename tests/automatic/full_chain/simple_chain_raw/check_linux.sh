#!/usr/bin/env bash

set -e

trap Cleanup EXIT

beamtime_id=11111111

monitor_database_name=db_test
proxy_address=127.0.0.1:8400


mkdir -p /tmp/asapo/asap3/petra3/gpfs/p07/2019/data/11111111
cp beamtime-metadata* /tmp/asapo/asap3/petra3/gpfs/p07/2019/data/11111111
mkdir -p /tmp/asapo/beamline/p07/current
cp beamtime-metadata* /tmp/asapo/beamline/p07/current/

Cleanup() {
    echo cleanup
    rm -rf out /tmp/asapo/asap3 /tmp/asapo/beamline
    echo "db.dropDatabase()" | mongo ${beamtime_id}_detector

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

echo "db.dropDatabase()" | mongo ${beamtime_id}_detector

token=`$3 token -endpoint http://localhost:8400/asapo-authorizer -secret admin_token.key -types read $beamtime_id`

#producer
$1 localhost:8400 ${beamtime_id} 100 10 4 100 100

$2 ${proxy_address} "__" ${beamtime_id} 2 $token 2000 1 > out
cat out
cat out   | grep "Processed 10 file(s)"
ls /tmp/asapo/beamline/p07/current/raw/1 | tee /dev/stderr | grep 1
