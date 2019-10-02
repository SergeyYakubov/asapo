#!/usr/bin/env bash

source_path=dummy
beamtime_id=test_run
stream=detector
database_name=${beamtime_id}_${stream}
token_test_run=K38Mqc90iRv8fC7prcFHd994mF_wfUiJnWBfIjIzieo=
group_id=bif31l2uiddd4r0q6b40
set -e



trap Cleanup EXIT

Cleanup() {
    set +e
    nomad stop nginx
    nomad run nginx_kill.nmd  && nomad stop -yes -purge nginx_kill
    nomad stop discovery
    nomad stop broker
	echo "db.dropDatabase()" | mongo ${database_name}
}

nomad run nginx.nmd
nomad run discovery.nmd
nomad run broker.nmd

for i in `seq 1 3`;
do
	echo 'db.data.insert({"_id":'$i',"size":100,"name":"'$i'","lastchange":1,"source":"none","buf_id":0,"meta":{"test":10}})' | mongo ${database_name}
done

echo 'db.meta.insert({"_id":0,"meta_test":"test"})' | mongo ${database_name}

sleep 1

export PYTHONPATH=$1:${PYTHONPATH}

python getnext.py 127.0.0.1:8400 $source_path $beamtime_id $token_test_run $group_id > out
cat out
cat out | grep '"size": 100'
cat out | grep '"_id": 1'
cat out | grep '"meta_test": "test"'

python getnext.py 127.0.0.1:8400 $source_path $beamtime_id $token_test_run $group_id> out
cat out
cat out | grep '"_id": 2'

python3 getnext.py 127.0.0.1:8400 $source_path $beamtime_id $token_test_run $group_id> out
cat out
cat out | grep '"_id": 3'


python3 getnext.py 127.0.0.1:8400 $source_path $beamtime_id $token_test_run new> out
cat out
cat out | grep '"_id": 1'


#echo $?



