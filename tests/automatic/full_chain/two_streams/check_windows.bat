SET mongo_exe="c:\Program Files\MongoDB\Server\3.6\bin\mongo.exe"
SET beamtime_id=asapo_test
SET beamline=test
SET stream1=s1
SET stream2=s2

SET receiver_root_folder=c:\tmp\asapo\receiver\files
SET receiver_folder="%receiver_root_folder%\%beamline%\%beamtime_id%"

"%3" token -secret broker_secret.key %beamtime_id% > token
set /P token=< token

set proxy_address="127.0.0.1:8400"

echo db.%beamtime_id%_%stream1%.insert({dummy:1}) | %mongo_exe% %beamtime_id%_%stream1%
echo db.%beamtime_id%_%stream2%.insert({dummy:1}) | %mongo_exe% %beamtime_id%_%stream2%


c:\opt\consul\nomad run receiver.nmd
c:\opt\consul\nomad run authorizer.nmd
c:\opt\consul\nomad run discovery.nmd
c:\opt\consul\nomad run broker.nmd
c:\opt\consul\nomad run nginx.nmd

ping 1.0.0.0 -n 10 -w 100 > nul

REM producer
mkdir %receiver_folder%
start /B "" "%1" %proxy_address% %beamtime_id%%%%stream1% 100 1000 4 0 100
start /B "" "%1" %proxy_address% %beamtime_id%%%%stream2% 100 900 4 0 100
ping 1.0.0.0 -n 1 -w 100 > nul

REM worker
"%2" %proxy_address% %receiver_folder% %beamtime_id%%%%stream1% 2 %token% 10000  0 > out1.txt
type out1.txt
findstr /i /l /c:"Processed 1000 file(s)"  out1.txt || goto :error

"%2" %proxy_address% %receiver_folder% %beamtime_id%%%%stream2% 2 %token% 10000  0 > out2.txt
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
echo db.dropDatabase() | %mongo_exe% %beamtime_id%_%stream1%
echo db.dropDatabase() | %mongo_exe% %beamtime_id%_%stream2%


