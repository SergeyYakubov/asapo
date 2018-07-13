#!/usr/bin/env bash

mongoaddr=`dig +short @127.0.0.1 mongo-config-srv.service.asapo | head -1`
cont=$(ssh $mongoaddr bash -c "'docker ps | grep mongo-config-srv'")
image=`echo "${cont##* }"`
ssh $mongoaddr docker exec $image bash -c "'echo \"rs.initiate()\" | mongo --host $mongoaddr --port 27010'"


mongoaddr=`dig +short @127.0.0.1 mongos-broker.service.asapo | head -1`
cont=$(ssh $mongoaddr bash -c "'docker ps | grep mongos'")
image=`echo "${cont##* }"`

shardaddr1=`dig +short @127.0.0.1 mongo-shard1.service.asapo | head -1`
shardaddr2=`dig +short @127.0.0.1 mongo-shard2.service.asapo | head -1`
shardaddr3=`dig +short @127.0.0.1 mongo-shard3.service.asapo | head -1`


ssh $mongoaddr docker exec $image bash -c "'echo \"sh.addShard('\"'$shardaddr1:27011'\"')\" | mongo --host $mongoaddr --port 27016'"
ssh $mongoaddr docker exec $image bash -c "'echo \"sh.addShard('\"'$shardaddr2:27012'\"')\" | mongo --host $mongoaddr --port 27016 '"
ssh $mongoaddr docker exec $image bash -c "'echo \"sh.addShard('\"'$shardaddr3:27013'\"')\" | mongo --host $mongoaddr --port 27016 '"
