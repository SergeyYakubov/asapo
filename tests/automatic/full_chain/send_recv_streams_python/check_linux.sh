#!/usr/bin/env bash

source_path=.
beamtime_id=asapo_test
stream_in=detector

indatabase_name=${beamtime_id}_${stream_in}
token=eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJqdGkiOiJjMTRhcDQzaXB0M2E0bmNpMDkwMCIsInN1YiI6ImJ0X2FzYXBvX3Rlc3QiLCJFeHRyYUNsYWltcyI6eyJBY2Nlc3NUeXBlIjoicmVhZCJ9fQ.X5Up3PBd81i4X7wUBXGkIrLEVSL-WO9kijDtzOqasgg

beamline=test

set -e

trap Cleanup EXIT

Cleanup() {
    set +e
    nomad stop nginx
    nomad run nginx_kill.nmd  && nomad stop -yes -purge nginx_kill
    nomad stop discovery
    nomad stop broker
    nomad stop receiver
    nomad stop authorizer
	echo "db.dropDatabase()" | mongo ${indatabase_name}
}

nomad run nginx.nmd
nomad run discovery.nmd
nomad run broker.nmd
nomad run receiver_tcp.nmd
nomad run authorizer.nmd


export PYTHONPATH=$2:$3:${PYTHONPATH}


$1 $4 127.0.0.1:8400 $beamtime_id $token | tee out
