#!/usr/bin/env bash

if [[ -z "$ASAPO_DOCKER_REPOSITORY" ]]; then
  ASAPO_DOCKER_REPOSITORY=yakser
fi

if [[ -z "$ASAPO_DOCKER_DO_PUSH" ]]; then
  ASAPO_DOCKER_DO_PUSH=YES
fi
