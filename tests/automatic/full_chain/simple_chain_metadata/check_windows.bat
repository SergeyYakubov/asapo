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
"%1" %proxy_address% %beamtime_id% 100 0 1 0 1000

REM consumer
"%2" %proxy_address% %receiver_folder% %beamtime_id% 2 %token% 5000  1 > out.txt
type out.txt
findstr /i /l /c:"dummy_meta"  out.txt || goto :error


goto :clean

:error
call :clean
exit /b 1

:clean
call stop_services.bat
rmdir /S /Q %receiver_root_folder%
del /f token
echo db.dropDatabase() | %mongo_exe% %beamtime_id%_detector


