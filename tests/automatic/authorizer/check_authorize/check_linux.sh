#!/usr/bin/env bash

set -e

trap Cleanup EXIT

Cleanup() {
	echo cleanup
	kill -9 $authorizeid
}

$@ -config settings.json  > authorizer.log &

sleep 0.3
authorizeid=`echo $!`

curl -v --silent --data '{"BeamtimeId":"c20180508-000-COM20181","OriginHost":"127.0.0.1:5555"}' 127.0.0.1:5007/authorize --stderr -

