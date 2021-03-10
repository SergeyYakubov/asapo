SET mongo_exe="c:\Program Files\MongoDB\Server\4.2\bin\mongo.exe"
SET beamtime_id=asapo_test
SET beamline=test
SET receiver_root_folder=c:\tmp\asapo\receiver\files
SET receiver_folder="%receiver_root_folder%\test_facility\gpfs\%beamline%\2019\data\%beamtime_id%"

set proxy_address="127.0.0.1:8400"

echo db.%beamtime_id%_detector.insert({dummy:1}) | %mongo_exe% %beamtime_id%_detector

call start_services.bat

"%3" token -endpoint http://127.0.0.1:8400/asapo-authorizer -secret admin_token.key -type read %beamtime_id% > token
set /P token=< token

REM producer
mkdir %receiver_folder%
start /B "" "%1" %proxy_address% %beamtime_id% 100 100 4 0 100 5
ping 192.0.2.1 -n 1 -w 1000 > nul

REM consumer
"%2" %proxy_address% %receiver_folder% %beamtime_id% 2 %token% 5000 1 1 > out.txt
type out.txt
findstr /i /l /c:"Processed 100 dataset(s)"  out.txt || goto :error
findstr /i /l /c:"with 500 file(s)"  out.txt || goto :error


goto :clean

:error
call :clean
exit /b 1

:clean
call stop_services.bat
rmdir /S /Q %receiver_root_folder%
del /f token
echo db.dropDatabase() | %mongo_exe% %beamtime_id%_detector


