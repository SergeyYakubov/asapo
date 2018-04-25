#!/usr/bin/env bash

set -e


rm -f $HOME/fluentd/asapo.*.log

$1

sleep 2

cat $HOME/fluentd/asapo.*.log

res=`cat $HOME/fluentd/asapo.*.log`

echo $res | grep "test_info"
echo $res | grep "test_error"
echo $res | grep "test_debug"
echo $res | grep "test_warning"

