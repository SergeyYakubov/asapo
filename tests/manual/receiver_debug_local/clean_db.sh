#!/usr/bin/env bash

beamtime_id=asapo_test


echo "db.dropDatabase()" | mongo ${beamtime_id}_detector
