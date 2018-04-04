#!/usr/bin/env bash

database_name=test_run

#set -e

$@ 0.0.0.0 1 1 2>&1 | grep "refused"

