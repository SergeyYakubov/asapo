SET mongo_exe="c:\Program Files\MongoDB\Server\3.6\bin\mongo.exe"
SET beamline=test
SET receiver_root_folder="c:\tmp\asapo\receiver\files"
SET receiver_folder="%receiver_root_folder%\%beamline%\%beamtime_id%"



set proxy_address="127.0.0.1:8400"

echo db.%beamtime_id%.insert({dummy:1}) | %mongo_exe% %beamtime_id%

c:\opt\consul\nomad run receiver.nmd
c:\opt\consul\nomad run authorizer.nmd
c:\opt\consul\nomad run discovery.nmd
c:\opt\consul\nomad run broker.nmd
c:\opt\consul\nomad run nginx.nmd

ping 1.0.0.0 -n 10 -w 100 > nul

REM producer
mkdir %receiver_folder%
start /B "" "%1" %proxy_address% %beamtime_id% 100 1000 4 0 100
ping 1.0.0.0 -n 1 -w 100 > nul

REM worker
"%2" %proxy_address% %beamtime_id% 2 | findstr /c:"Processed 1000 file(s)"  || goto :error


goto :clean

:error
call :clean
exit /b 1

:clean
c:\opt\consul\nomad stop receiver
c:\opt\consul\nomad stop discovery
c:\opt\consul\nomad stop broker
c:\opt\consul\nomad stop authorizer
c:\opt\consul\nomad stop nginx
rmdir /S /Q %receiver_root_folder%
echo db.dropDatabase() | %mongo_exe% %beamtime_id%


