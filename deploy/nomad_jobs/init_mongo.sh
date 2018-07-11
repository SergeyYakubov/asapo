#!/usr/bin/env bash

mongoaddr=`dig +short @127.0.0.1 mongo-config-srv.service.asapo | head -1`
cont=$(ssh $mongoaddr bash -c "'docker ps | grep mongo-config-srv'")
image=`echo "${cont##* }"`
ssh $mongoaddr docker exec $image bash -c "'echo \"rs.initiate()\" | mongo --port 27010'"


mongoaddr=`dig +short @127.0.0.1 mongos.service.asapo | head -1`
cont=$(ssh $mongoaddr bash -c "'docker ps | grep mongos'")
image=`echo "${cont##* }"`

shardaddr=`dig +short @127.0.0.1 mongo-shard1.service.asapo | head -1`


ssh $mongoaddr docker exec $image bash -c "'echo \"sh.addShard('\"'$shardaddr:27011'\"')\" | mongo'"

