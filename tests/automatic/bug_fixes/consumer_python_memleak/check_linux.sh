#!/usr/bin/env bash
set -e

trap Cleanup EXIT

endpoint=127.0.0.1:8400
path=.
beamtime_id=asapo_test
token="IEfwsWa0GXky2S3MkxJSUHJT1sI8DD5teRdjBUXVRxk="


Cleanup() {
    echo cleanup
    rm -rf $fname
    nomad stop nginx
    nomad run nginx_kill.nmd  && nomad stop -yes -purge nginx_kill
    nomad stop broker
    nomad stop discovery
    echo "db.dropDatabase()" | mongo ${beamtime_id}_stream
}

fname=test.dat
size=100000000 # 10MB

nomad run nginx.nmd
nomad run discovery.nmd
nomad run broker.nmd

sleep 1

echo 'db.data_default.insert({"_id":'1',"size":'$size',"name":"'$fname'","lastchange":1,"source":"none","buf_id":0,"meta":{}})' | mongo ${beamtime_id}_stream
dd if=/dev/zero of=$fname bs=$size count=1

export PYTHONPATH=$1:${PYTHONPATH}
export Python_EXECUTABLE=$2


$Python_EXECUTABLE memleak.py $endpoint $path $beamtime_id $token &> out &
pid=`echo $!`

sleep 1

mem1=`pmap -x $pid  | tail -n 1 | awk  '{print $3}'`

sleep 3

mem2=`pmap -x $pid  | tail -n 1 | awk  '{print $3}'`

kill -9 $pid

leak=$(( $mem2 - $mem1 ))

cat out
echo leak: $leak

test $leak -eq 0