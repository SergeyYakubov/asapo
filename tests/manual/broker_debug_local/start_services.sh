#!/usr/bin/env bash

nomad run authorizer.nmd
nomad run discovery.nmd
#nomad run broker.nmd
nomad run receiver_tcp.nmd
nomad run nginx.nmd
