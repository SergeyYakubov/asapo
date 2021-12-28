#!/usr/bin/env bash

set -e

trap Cleanup EXIT

database_name=db_test
beamtime_id=asapo_test
beamline=test
receiver_root_folder=/tmp/asapo/receiver/files
facility=test_facility
year=2019
receiver_folder=${receiver_root_folder}/${facility}/gpfs/${beamline}/${year}/data/${beamtime_id}


Cleanup() {
    echo cleanup
    set +e
    nomad stop receiver
    nomad run receiver_tcp.nmd
    while true
    do
      sleep 1
      curl --silent 127.0.0.1:8400/asapo-discovery/v0.1/asapo-receiver?protocol=v0.1 --stderr - | grep 127.0.0.1  || continue
      echo recevier started
      break
    done
	rm -rf ${receiver_root_folder}
    echo "db.dropDatabase()" | mongo ${beamtime_id}_detector
    influx -database ${database_name} -execute "drop series from statistics, RequestsRate"
}

./transfer-single-file_kafka & KAFKA_PID=$!

echo "Started the kafka listener"

while [ ! -f bootstrap ]; do
    if ! kill -0 $KAFKA_PID > /dev/null 2>&1; then
        # listener exited preliminary, i.e. some error
        exit 1
    fi
    sleep 1
done

BOOTSTRAP=$(cat bootstrap)

echo "Read kafka bootstrap: ${BOOTSTRAP}"

export NOMAD_ADDR_bootstrap=${BOOTSTRAP}

nomad stop receiver
nomad run receiver_kafka.nmd
while true
do
  sleep 1
  curl --silent 127.0.0.1:8400/asapo-discovery/v0.1/asapo-receiver?protocol=v0.1 --stderr - | grep 127.0.0.1  || continue
  echo recevier started
  break
done

mkdir -p ${receiver_folder}

$1 localhost:8400 ${beamtime_id} 100 1 1  0 30

ls -ln ${receiver_folder}/processed/1 | awk '{ print $5 }'| grep 100000

$1 localhost:8400 wrong_beamtime_id 100 1 1 0 1 2>&1 | tee /dev/stderr | grep "authorization"

wait $KAFKA_PID
RESULT=$?

echo "Mock kafka returned $RESULT"
