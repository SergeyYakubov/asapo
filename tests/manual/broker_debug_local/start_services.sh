#!/usr/bin/env bash

nomad run authorizer.nmd
nomad run discovery.nmd
#nomad run broker.nmd
nomad run receiver.nmd
nomad run nginx.nmd
