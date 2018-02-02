#!/usr/bin/env bash

database_name=test_run

set -e

trap Cleanup EXIT

Cleanup() {
:
	kill -9 $brokerid
	echo "db.dropDatabase()" | mongo ${database_name}
}

args=${@:1:$(($# - 1))}
broker=${@:$#}

$broker &
brokerid=`echo $!`
sleep 0.3

for i in `seq 1 3`;
do
	echo 'db.data.insert({"_id":'$i',"size":100,"base_name":"'$i'","lastchange":1,"relative_path":""})' | mongo ${database_name}
done

$args 127.0.0.1:5005 $database_name 2 | grep "Processed 3 file(s)"

