#!/usr/bin/env bash

set -e

trap Cleanup EXIT

beamtime_id=11111111

monitor_database_name=db_test
proxy_address=127.0.0.1:8400

mkdir -p asap3/petra3/gpfs/p07/2019/data/11111111
cp beamtime-metadata* asap3/petra3/gpfs/p07/2019/data/11111111
mkdir -p beamline/p07/current
cp beamtime-metadata* beamline/p07/current/

Cleanup() {
    echo cleanup
    nomad stop nginx
    nomad run nginx_kill.nmd  && nomad stop -yes -purge nginx_kill
    nomad stop receiver
    nomad stop discovery
    nomad stop broker
    nomad stop authorizer
    rm -rf out asap3 beamline
    echo "db.dropDatabase()" | mongo ${beamtime_id}_detector
    influx -execute "drop database ${monitor_database_name}"
}

#echo "db.data_${beamtime_id}_detector.insert({dummy:1})" | mongo data_${beamtime_id}_detector
echo "db.dropDatabase()" | mongo ${beamtime_id}_detector


nomad run nginx.nmd
nomad run authorizer.nmd
nomad run receiver_tcp.nmd
nomad run discovery.nmd
nomad run broker.nmd

sleep 1

token=`$3 token -endpoint http://localhost:8400/asapo-authorizer -secret admin_token.key -type read $beamtime_id`

#producer
$1 localhost:8400 ${beamtime_id} 100 10 4 100 100


$2 ${proxy_address} "__" ${beamtime_id} 2 $token 5000 1 > out
cat out
cat out   | grep "Processed 10 file(s)"
ls beamline/p07/current/raw/1 | tee /dev/stderr | grep 1
