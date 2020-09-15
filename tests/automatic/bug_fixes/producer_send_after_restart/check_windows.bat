

SET mongo_exe="c:\Program Files\MongoDB\Server\4.2\bin\mongo.exe"
SET beamtime_id=asapo_test
SET beamline=test
SET receiver_root_folder=c:\tmp\asapo\receiver\files
SET receiver_folder="%receiver_root_folder%\test_facility\gpfs\%beamline%\2019\data\%beamtime_id%"

set producer_short_name="%~nx1"


"%3" token -secret auth_secret.key %beamtime_id% > token
set /P token=< token

set proxy_address="127.0.0.1:8400"

echo db.%beamtime_id%_detector.insert({dummy:1}) | %mongo_exe% %beamtime_id%_detector

call start_services.bat

REM producer
mkdir %receiver_folder%
mkdir  c:\tmp\asapo\test_in\processed
start /B "" "%1" test.json

ping 1.0.0.0 -n 3 -w 100 > nul

echo hello > c:\tmp\asapo\test_in\processed\file1
echo hello > c:\tmp\asapo\test_in\processed\file2

ping 1.0.0.0 -n 3 -w 100 > nul

c:\opt\consul\nomad stop receiver
c:\opt\consul\nomad run receiver.nmd

ping 1.0.0.0 -n 3 -w 100 > nul
ping 1.0.0.0 -n 3 -w 100 > nul
ping 1.0.0.0 -n 10 -w 100 > nul


echo hello > c:\tmp\asapo\test_in\processed\file3

ping 1.0.0.0 -n 10 -w 100 > nul


REM consumer
"%2" %proxy_address% %receiver_folder% %beamtime_id% 2 %token% 3000 1 | findstr /c:"Processed 3 file(s)"  || goto :error


goto :clean

:error
call :clean
exit /b 1

:clean
call stop_services.bat

rmdir /S /Q %receiver_root_folder%
rmdir /S /Q c:\tmp\asapo\test_in
Taskkill /IM "%producer_short_name%" /F

del /f token
echo db.dropDatabase() | %mongo_exe% %beamtime_id%_detector



