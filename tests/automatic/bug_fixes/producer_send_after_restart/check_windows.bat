SET mongo_exe="c:\Program Files\MongoDB\Server\4.2\bin\mongo.exe"
SET beamtime_id=asapo_test
SET beamline=test
SET receiver_root_folder=c:\tmp\asapo\receiver\files
SET receiver_folder="%receiver_root_folder%\test_facility\gpfs\%beamline%\2019\data\%beamtime_id%"

set producer_short_name="%~nx1"

set proxy_address="127.0.0.1:8400"

"%3" token -endpoint http://127.0.0.1:8400/asapo-authorizer -secret admin_token.key -types read %beamtime_id% > token
set /P token=< token

REM producer
mkdir %receiver_folder%
mkdir  c:\tmp\asapo\test_in\processed
start /B "" "%1" test.json

ping 192.0.2.1 -n 1 -w 1000 > nul

echo hello > c:\tmp\asapo\test_in\processed\file1
echo hello > c:\tmp\asapo\test_in\processed\file2

ping 192.0.2.1 -n 3 -w 1000 > nul

c:\opt\consul\nomad stop receiver
ping 192.0.2.1 -n 1 -w 1000 > nul
c:\opt\consul\nomad run receiver_tcp.nmd
echo hello > c:\tmp\asapo\test_in\processed\file3

ping 192.0.2.1 -n 10 -w 1000 > nul

REM consumer
"%2" %proxy_address% %receiver_folder% %beamtime_id% 2 %token% 3000 1 | findstr /c:"Processed 3 file(s)"  || goto :error

goto :clean

:error
call :clean
exit /b 1

:clean
rmdir /S /Q %receiver_root_folder%
rmdir /S /Q c:\tmp\asapo\test_in
Taskkill /IM "%producer_short_name%" /F

del /f token
echo db.dropDatabase() | %mongo_exe% %beamtime_id%_detector



