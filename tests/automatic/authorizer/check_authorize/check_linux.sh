#!/usr/bin/env bash

set -e

trap Cleanup EXIT

Cleanup() {
	echo cleanup
}

mkdir -p /tmp/asapo/asap3/petra3/gpfs/p00/2019/comissioning/c20180508-000-COM20181
mkdir -p /tmp/asapo/asap3/petra3/gpfs/p00/2019/data/11000015
mkdir -p /tmp/asapo/beamline/p07/current
mkdir -p /tmp/asapo/beamline/p08/current
cp beamtime-metadata-11111111.json /tmp/asapo/beamline/p07/current/
cp beamtime-metadata-11111112.json /tmp/asapo/beamline/p08/current/


#tokens
AdminToken=$ASAPO_CREATE_TOKEN
echo admin $AdminToken

curl -v --silent -H "Authorization: Bearer $AdminToken" --data '{"Subject": {"beamtimeId":"12345678"},"DaysValid":123,"AccessType":["read"]}' 127.0.0.1:8400/asapo-authorizer/admin/issue --stderr -  | tee /dev/stderr | grep "bt_12345678"
curl -v --silent -H "Authorization: Bearer blabla" --data '{"Subject": {"beamtimeId":"12345678"},"DaysValid":123,"AccessType":["read"]}' 127.0.0.1:8400/asapo-authorizer/admin/issue --stderr -  | tee /dev/stderr | grep "token does not match"

curl -v --silent --data '{"SourceCredentials":"processed%c20180508-000-COM20181%%detector%","OriginHost":"127.0.0.1:5555"}' 127.0.0.1:8400/asapo-authorizer/authorize --stderr -  | tee /dev/stderr  | grep c20180508-000-COM20181
curl -v --silent --data '{"SourceCredentials":"processed%c20180508-000-COM20181%%detector%","OriginHost":"127.0.0.1:5555"}' 127.0.0.1:8400/asapo-authorizer/authorize --stderr -  | tee /dev/stderr  | grep p00
curl -v --silent --data '{"SourceCredentials":"processed%c20180508-000-COM20181%%detector%","OriginHost":"127.0.0.1:5555"}' 127.0.0.1:8400/asapo-authorizer/authorize --stderr -  | tee /dev/stderr  | grep detector

token=$C20180508_000_COM20181_TOKEN

curl -v --silent --data "{\"SourceCredentials\":\"processed%c20180508-000-COM20181%%detector%$token\",\"OriginHost\":\"bla\"}" 127.0.0.1:8400/asapo-authorizer/authorize --stderr -  | tee /dev/stderr  | grep detector
curl -v --silent --data "{\"SourceCredentials\":\"processed%c20180508-000-COM20181%auto%detector%$token\",\"OriginHost\":\"bla\"}" 127.0.0.1:8400/asapo-authorizer/authorize --stderr -  | tee /dev/stderr  | grep p00
curl -v --silent --data '{"SourceCredentials":"processed%c20180508-000-COM20181%%detector%bla","OriginHost":"bla"}' 127.0.0.1:8400/asapo-authorizer/authorize --stderr -  | tee /dev/stderr  | grep 401

token=$BT11000015_TOKEN
#beamtine not online
curl -v --silent --data "{\"SourceCredentials\":\"raw%11000015%%detector%$token\",\"OriginHost\":\"bla\"}" 127.0.0.1:8400/asapo-authorizer/authorize --stderr -  | tee /dev/stderr  | grep 401

token=$BT11000016_TOKEN
curl -v --silent --data "{\"SourceCredentials\":\"raw%11000016%%detector%${token}\",\"OriginHost\":\"bla\"}" 127.0.0.1:8400/asapo-authorizer/authorize --stderr -  | tee /dev/stderr  | grep 401


token=$BLP07_TOKEN

curl -v --silent --data "{\"SourceCredentials\":\"processed%auto%p07%detector%$token\",\"OriginHost\":\"bla\"}" 127.0.0.1:8400/asapo-authorizer/authorize --stderr -  | tee /dev/stderr  | grep 11111111
curl -v --silent --data "{\"SourceCredentials\":\"raw%auto%p07%detector%\",\"OriginHost\":\"127.0.0.1:8400/asapo-authorizer\"}" 127.0.0.1:8400/asapo-authorizer/authorize --stderr -  | tee /dev/stderr  | grep writeraw
curl -v --silent --data "{\"SourceCredentials\":\"raw%auto%p07%detector%$token\",\"OriginHost\":\"127.0.0.1:8400/asapo-authorizer\"}" 127.0.0.1:8400/asapo-authorizer/authorize --stderr -  | tee /dev/stderr  | not grep writeraw
curl -v --silent --data "{\"SourceCredentials\":\"raw%auto%p07%detector%$token\",\"OriginHost\":\"127.0.0.1:8400/asapo-authorizer\"}" 127.0.0.1:8400/asapo-authorizer/authorize --stderr -  | tee /dev/stderr  | grep p07
curl -v --silent --data "{\"SourceCredentials\":\"raw%auto%p07%detector%$token\",\"OriginHost\":\"127.0.0.1:8400/asapo-authorizer\"}" 127.0.0.1:8400/asapo-authorizer/authorize --stderr -  | tee /dev/stderr  | grep /asap3/petra3/gpfs/p07/2020/data/11111111

#wrong data in metafile
curl -v --silent --data "{\"SourceCredentials\":\"processed%auto%p08%detector%$token\",\"OriginHost\":\"bla\"}" 127.0.0.1:8400/asapo-authorizer/authorize --stderr -  | tee /dev/stderr  | grep "cannot set meta fields"

#read access
curl -v --silent --data "{\"SourceCredentials\":\"processed%auto%p07%detector%$token\",\"OriginHost\":\"bla\"}" 127.0.0.1:8400/asapo-authorizer/authorize --stderr - | tee /dev/stderr  | grep read

#write access
token=$BLP07_W_TOKEN
curl -v --silent --data "{\"SourceCredentials\":\"processed%auto%p07%detector%$token\",\"OriginHost\":\"bla\"}" 127.0.0.1:8400/asapo-authorizer/authorize --stderr -  | tee /dev/stderr  | grep write

rm -rf /tmp/asapo/asap3 /tmp/asapo/beamline