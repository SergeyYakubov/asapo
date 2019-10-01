#!/usr/bin/env bash

nomad stop authorizer
nomad stop discovery
nomad stop nginx
nomad run nginx_kill.nmd  && nomad stop -yes -purge nginx_kill
nomad stop receiver
