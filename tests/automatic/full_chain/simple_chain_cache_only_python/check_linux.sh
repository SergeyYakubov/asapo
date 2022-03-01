#!/usr/bin/env bash

set -e

trap Cleanup EXIT

database_name=db_test
data_source=python
beamtime_id=asapo_test

Cleanup() {
	echo cleanup
  echo "db.dropDatabase()" | mongo ${beamtime_id}_${data_source}
  influx -database ${database_name} -execute "drop series from statistics, RequestsRate"
}

export PYTHONPATH=$2:$3:${PYTHONPATH}

$1 $4 $data_source $beamtime_id  "127.0.0.1:8400" $ASAPO_TEST_RW_TOKEN &> out
cat out
