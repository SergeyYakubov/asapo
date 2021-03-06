#!/usr/bin/env bash
set -e

trap Cleanup EXIT

endpoint=127.0.0.1:8400
path=.
beamtime_id=asapo_test
#asapo_test read token
token=$ASAPO_TEST_RW_TOKEN


Cleanup() {
    echo cleanup
    rm -rf $fname
    echo "db.dropDatabase()" | mongo ${beamtime_id}_stream
}

fname=test.dat
size=100000000 # 10MB


echo 'db.data_default.insert({"_id":'1',"size":'$size',"name":"'$fname'","timestamp":1,"source":"none","buf_id":0,"dataset_substream":0,"meta":{}})' | mongo ${beamtime_id}_stream
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

test $leak -lt 300000
