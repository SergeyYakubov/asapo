#!/usr/bin/env bash

database_name=test_run

#set -e


#just test that it starts, no reciever is running
$@ 0.0.0.0 1 1 2>&1 | grep "refused"

