SET mongo_exe="c:\Program Files\MongoDB\Server\4.2\bin\mongo.exe"
SET beamtime_id=asapo_test
SET beamline=test
SET receiver_root_folder=c:\tmp\asapo\receiver\files
SET receiver_folder="%receiver_root_folder%\test_facility\gpfs\%beamline%\2019\data\%beamtime_id%"


"%2" token -secret auth_secret.key %beamtime_id% > token
set /P token=< token

set proxy_address="127.0.0.1:8400"

echo db.%beamtime_id%_detector.insert({dummy:1}) | %mongo_exe% %beamtime_id%_detector

call start_services.bat

REM producer
mkdir %receiver_folder%
"%1" %proxy_address% %beamtime_id% 100 100 4 0 100

REM consumer
set PYTHONPATH=%4

python3 %3/get_user_meta.py %proxy_address% "%6" %receiver_folder% %beamtime_id%  %token% new > out
type out
type out | findstr /c:"found images: 100" || goto :error
type out | findstr /c:"test100" || goto :error

goto :clean

:error
call :clean
exit /b 1

:clean
call stop_services.bat
rmdir /S /Q %receiver_root_folder%
del /f token
echo db.dropDatabase() | %mongo_exe% %beamtime_id%_detector


