#!/usr/bin/env bash

nomad run asapo-nginx.nmd
nomad run asapo-logging.nmd
nomad run asapo-mongo.nmd
nomad run asapo-brokers.nmd
nomad run asapo-services.nmd
nomad run asapo-perfmetrics.nmd
nomad run asapo-receivers.nmd
