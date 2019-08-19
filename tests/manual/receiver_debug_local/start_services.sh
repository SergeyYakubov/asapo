#!/usr/bin/env bash

nomad run authorizer.nmd
nomad run discovery.nmd
nomad run nginx.nmd
