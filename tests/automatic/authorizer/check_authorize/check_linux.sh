#!/usr/bin/env bash

set -e

trap Cleanup EXIT

Cleanup() {
	echo cleanup
	kill -9 $authorizeid
	sleep 1
}

$@ -config settings.json &
sleep 1
authorizeid=`echo $!`

mkdir -p asap3/petra3/gpfs/p00/2019/comissioning/c20180508-000-COM20181
mkdir -p asap3/petra3/gpfs/p00/2019/data/11000015
mkdir -p beamline/p07/current
cp beamtime-metadata* beamline/p07/current/

#tokens
AdminToken=eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJqdGkiOiJjMTNvcGpyaXB0MzNlb2ZjbWJuZyIsInN1YiI6ImFkbWluIiwiRXh0cmFDbGFpbXMiOnsiQWNjZXNzVHlwZSI6ImNyZWF0ZSJ9fQ.uRjtGPaRpOlOfKroijHRgMDNaZHnXsVPf0JaJ1XMg7o
curl -v --silent -H "Authorization: Bearer $AdminToken" --data '{"Subject": {"beamtimeId":"12345678"},"DaysValid":123,"AccessType":["read"]}' 127.0.0.1:5007/admin/issue --stderr -  | tee /dev/stderr | grep "bt_12345678"
curl -v --silent -H "Authorization: Bearer blabla" --data '{"Subject": {"beamtimeId":"12345678"},"DaysValid":123,"AccessType":["read"]}' 127.0.0.1:5007/admin/issue --stderr -  | tee /dev/stderr | grep "token does not match"

curl -v --silent --data '{"SourceCredentials":"processed%c20180508-000-COM20181%%detector%","OriginHost":"127.0.0.1:5555"}' 127.0.0.1:5007/authorize --stderr -  | tee /dev/stderr  | grep c20180508-000-COM20181
curl -v --silent --data '{"SourceCredentials":"processed%c20180508-000-COM20181%%detector%","OriginHost":"127.0.0.1:5555"}' 127.0.0.1:5007/authorize --stderr -  | tee /dev/stderr  | grep p00
curl -v --silent --data '{"SourceCredentials":"processed%c20180508-000-COM20181%%detector%","OriginHost":"127.0.0.1:5555"}' 127.0.0.1:5007/authorize --stderr -  | tee /dev/stderr  | grep detector

token=eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJqdGkiOiJjMTNxZWpyaXB0MzUybHQxNjhyZyIsInN1YiI6ImJ0X2MyMDE4MDUwOC0wMDAtQ09NMjAxODEiLCJFeHRyYUNsYWltcyI6eyJBY2Nlc3NUeXBlIjoicmVhZCJ9fQ.MDuQa_f0yOcn35xIgiCfoVVT56oTQ5tSiuKu9VqO_tE #token for c20180508-000-COM20181

curl -v --silent --data "{\"SourceCredentials\":\"processed%c20180508-000-COM20181%%detector%$token\",\"OriginHost\":\"bla\"}" 127.0.0.1:5007/authorize --stderr -  | tee /dev/stderr  | grep detector
curl -v --silent --data "{\"SourceCredentials\":\"processed%c20180508-000-COM20181%auto%detector%$token\",\"OriginHost\":\"bla\"}" 127.0.0.1:5007/authorize --stderr -  | tee /dev/stderr  | grep p00
curl -v --silent --data '{"SourceCredentials":"processed%c20180508-000-COM20181%%detector%bla","OriginHost":"bla"}' 127.0.0.1:5007/authorize --stderr -  | tee /dev/stderr  | grep 401

token=eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJqdGkiOiJjMTNxYnZqaXB0MzR0cTNtMGM5ZyIsInN1YiI6ImJ0XzExMDAwMDE1IiwiRXh0cmFDbGFpbXMiOnsiQWNjZXNzVHlwZSI6InJlYWQifX0.oiweTX_mHIRHkX7_jfOJfHM8lncapROfdQlD7cR7_84 #token for 11000015
#beamtine not online
curl -v --silent --data "{\"SourceCredentials\":\"raw%11000015%%detector%$token\",\"OriginHost\":\"bla\"}" 127.0.0.1:5007/authorize --stderr -  | tee /dev/stderr  | grep 401

token=eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJqdGkiOiJjMTNxYzNqaXB0MzR0cjlyOWhiZyIsInN1YiI6ImJ0XzExMDAwMDE2IiwiRXh0cmFDbGFpbXMiOnsiQWNjZXNzVHlwZSI6InJlYWQifX0.2UxFNyI9rNwX9H0ErPNjJxZBy9WEv7CYq1N1d-93Jmg #token for 11000016
curl -v --silent --data "{\"SourceCredentials\":\"raw%11000016%%detector%${token}\",\"OriginHost\":\"bla\"}" 127.0.0.1:5007/authorize --stderr -  | tee /dev/stderr  | grep 401


token=eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJqdGkiOiJjMTNxcmFyaXB0MzVjcWpuMmUxZyIsInN1YiI6ImJsX3AwNyIsIkV4dHJhQ2xhaW1zIjp7IkFjY2Vzc1R5cGUiOiJyZWFkIn19.KQFj3hOJRpc7hPqwJyYmnQ31IrR1zSz4EifUuulmP5E # for beamlne p07
curl -v --silent --data "{\"SourceCredentials\":\"processed%auto%p07%detector%$token\",\"OriginHost\":\"bla\"}" 127.0.0.1:5007/authorize --stderr -  | tee /dev/stderr  | grep 11111111
curl -v --silent --data "{\"SourceCredentials\":\"raw%auto%p07%detector%$token\",\"OriginHost\":\"127.0.0.1:5007\"}" 127.0.0.1:5007/authorize --stderr -  | tee /dev/stderr  | grep 11111111
curl -v --silent --data "{\"SourceCredentials\":\"raw%auto%p07%detector%$token\",\"OriginHost\":\"127.0.0.1:5007\"}" 127.0.0.1:5007/authorize --stderr -  | tee /dev/stderr  | grep p07
curl -v --silent --data "{\"SourceCredentials\":\"raw%auto%p07%detector%$token\",\"OriginHost\":\"127.0.0.1:5007\"}" 127.0.0.1:5007/authorize --stderr -  | tee /dev/stderr  | grep /asap3/petra3/gpfs/p07/2020/data/11111111

#read access
curl -v --silent --data "{\"SourceCredentials\":\"processed%auto%p07%detector%$token\",\"OriginHost\":\"bla\"}" 127.0.0.1:5007/authorize --stderr - | tee /dev/stderr  | grep read

#write access
token=eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJqdGkiOiJjMTQ4MG1yaXB0Mzc2Z2xvNWo3MCIsInN1YiI6ImJsX3AwNyIsIkV4dHJhQ2xhaW1zIjp7IkFjY2Vzc1R5cGUiOiJ3cml0ZSJ9fQ.8e4Xo1w-ICJzKjOwj2sGtpVfGppSGgPHv1yPLJwsdSA # for beamlne p07, write access
curl -v --silent --data "{\"SourceCredentials\":\"processed%auto%p07%detector%$token\",\"OriginHost\":\"bla\"}" 127.0.0.1:5007/authorize --stderr -  | tee /dev/stderr  | grep write

rm -rf asap3 beamline