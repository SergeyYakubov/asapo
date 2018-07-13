#!/usr/bin/env bash

nomad run asapo-nginx.nmd
nomad run asapo-logging.nmd
nomad run asapo-mongo-sharded.nmd

#if mongo replica sets and shards not created yet might be necessary to wait here, than start mongos (run init_mongo.sh first time to create replicas,
#then start mongos (e.g. in asapo-brokers job), then run init_mongo.sh once more


nomad run asapo-brokers.nmd
nomad run asapo-services.nmd
nomad run asapo-perfmetrics.nmd
nomad run asapo-receivers.nmd
