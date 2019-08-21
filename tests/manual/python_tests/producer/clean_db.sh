#!/usr/bin/env bash

beamtime_id=asapo_test1


echo "db.dropDatabase()" | mongo ${beamtime_id}_detector
