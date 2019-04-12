SET mongo_exe="c:\Program Files\MongoDB\Server\3.6\bin\mongo.exe"
SET beamtime_id1=asapo_test1
SET beamline1=test1
SET beamtime_id2=asapo_test2
SET beamline2=test2

SET receiver_root_folder=c:\tmp\asapo\receiver\files
SET receiver_folder1="%receiver_root_folder%\%beamline1%\%beamtime_id1%"
SET receiver_folder2="%receiver_root_folder%\%beamline2%\%beamtime_id2%"


"%3" token -secret broker_secret.key %beamtime_id1% > token
set /P token1=< token
"%3" token -secret broker_secret.key %beamtime_id2% > token
set /P token2=< token

set proxy_address="127.0.0.1:8400"

echo db.%beamtime_id1%.insert({dummy:1}) | %mongo_exe% %beamtime_id1%
echo db.%beamtime_id2%.insert({dummy:1}) | %mongo_exe% %beamtime_id2%

c:\opt\consul\nomad run receiver.nmd
c:\opt\consul\nomad run authorizer.nmd
c:\opt\consul\nomad run discovery.nmd
c:\opt\consul\nomad run broker.nmd
c:\opt\consul\nomad run nginx.nmd

ping 1.0.0.0 -n 10 -w 100 > nul

REM producer
mkdir %receiver_folder1%
mkdir %receiver_folder2%
start /B "" "%1" %proxy_address% %beamtime_id1% 100 1000 4 0 100
start /B "" "%1" %proxy_address% %beamtime_id2% 100 900 4 0 100
ping 1.0.0.0 -n 1 -w 100 > nul

REM worker
"%2" %proxy_address% %receiver_folder1% %beamtime_id1% 2 %token1% 2000  0 > out1.txt
type out1.txt
findstr /i /l /c:"Processed 1000 file(s)"  out1.txt || goto :error

"%2" %proxy_address% %receiver_folder2% %beamtime_id2% 2 %token2% 2000  0 > out2.txt
type out2.txt
findstr /i /l /c:"Processed 900 file(s)"  out2.txt || goto :error



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
del /f token1
del /f token2
echo db.dropDatabase() | %mongo_exe% %beamtime_id1%
echo db.dropDatabase() | %mongo_exe% %beamtime_id2%


