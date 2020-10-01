

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

ping 192.0.2.1 -n 1 -w 1000 > nul

mkdir  c:\tmp\asapo\test_in\processed\test1
mkdir  c:\tmp\asapo\test_in\processed\test2


echo hello > c:\tmp\asapo\test_in\processed\test1\file1
echo hello > c:\tmp\asapo\test_in\processed\test1\file2
echo hello > c:\tmp\asapo\test_in\processed\test2\file2

ping 192.0.2.1 -n 1 -w 1000 > nul


REM consumer
"%2" %proxy_address% %receiver_folder% %beamtime_id% 2 %token% 1000  1 > out.txt
type out.txt
findstr /i /l /c:"Processed 3 file(s)"  out.txt || goto :error

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
