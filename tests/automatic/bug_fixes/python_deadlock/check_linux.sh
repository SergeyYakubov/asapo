#!/usr/bin/env bash
set -e

export PYTHONPATH=$2:${PYTHONPATH}
export Python3_EXECUTABLE=$3

$Python3_EXECUTABLE $1 $endpoint $beamtime_id $token


