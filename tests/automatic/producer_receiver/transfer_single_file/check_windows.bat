SET mongo_exe="c:\Program Files\MongoDB\Server\4.2\bin\mongo.exe"
SET beamtime_id=asapo_test
SET beamline=test
SET receiver_root_folder=c:\tmp\asapo\receiver\files
SET receiver_folder="%receiver_root_folder%\test_facility\gpfs\%beamline%\2019\data\%beamtime_id%"


echo db.%beamtime_id%_detector.insert({dummy:1})" | %mongo_exe% %beamtime_id%_detector

call start_services.bat

mkdir %receiver_folder%

"%1" localhost:8400 %beamtime_id% 100 1 1 0 30

ping 192.0.2.1 -n 1 -w 1000 > nul

FOR /F "usebackq" %%A IN ('%receiver_folder%\processed\1') DO set size=%%~zA
if %size% NEQ 100000 goto :error

"%1" localhost:8400 wrong_id 100 1 1 0 2 2>1 | findstr /c:"authorization"  || goto :error

goto :clean

:error
call :clean
exit /b 1

:clean
call stop_services.bat
rmdir /S /Q %receiver_root_folder%
echo db.dropDatabase() | %mongo_exe% %beamtime_id%_detector


