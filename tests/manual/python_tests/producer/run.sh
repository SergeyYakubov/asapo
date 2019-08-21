#!/usr/bin/env bash
export PYTHONPATH=/home/yakubov/projects/asapo/cmake-build-debug/producer/api/python:${PYTHONPATH}

mkdir -p /tmp/asapo/receiver/files/test1/asapo_test1

python test.py
