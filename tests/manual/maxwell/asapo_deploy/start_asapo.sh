#!/usr/bin/env bash

set -e

asapo_host=`cat asapo_host`

! ssh $asapo_host DOCKER_CONT_NAME=asapo dockerexec cat /var/nomad/token > token


ssh $asapo_host DOCKER_CONT_NAME=asapo dockerexec asapo-start $@
ssh $asapo_host DOCKER_CONT_NAME=asapo dockerexec cat /var/nomad/token > token

cat token