#!/usr/bin/env bash

mongoaddr=`dig +short @127.0.0.1 mongo.service.asapo | head -1`

database="asapo_test"

cont=$(ssh $mongoaddr bash -c "'docker ps | grep mongo'")

image=`echo "${cont##* }"`

rm -rf /bldocuments/support/asapo/data/test/asapo_test
rm -rf /bldocuments/support/asapo/data/test1/asapo_test1
rm -rf /bldocuments/support/asapo/data/test2/asapo_test2

mkdir /bldocuments/support/asapo/data/test/asapo_test
mkdir /bldocuments/support/asapo/data/test1/asapo_test1
mkdir /bldocuments/support/asapo/data/test2/asapo_test2
chown asapo: -R /bldocuments/support/asapo/data/test/

ssh $mongoaddr docker exec $image bash -c "'echo \"db.dropDatabase()\" | mongo --host $mongoaddr  asapo_test'"
ssh $mongoaddr docker exec $image bash -c "'echo \"db.dropDatabase()\" | mongo --host $mongoaddr  asapo_test1'"
ssh $mongoaddr docker exec $image bash -c "'echo \"db.dropDatabase()\" | mongo --host $mongoaddr  asapo_test2'"



nomad stop asapo-brokers
nomad run asapo-brokers.nmd