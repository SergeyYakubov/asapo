SET mongo_exe="c:\Program Files\MongoDB\Server\4.2\bin\mongo.exe"
SET beamtime_id1=asapo_test1
SET beamline1=test1
SET beamtime_id2=asapo_test2
SET beamline2=test2
SET data_source=detector

SET receiver_root_folder=c:\tmp\asapo\receiver\files
SET facility=test_facility
SET year=2019


SET receiver_folder1="%receiver_root_folder%\%facility%\gpfs\%beamline1%\%year%\data\%beamtime_id1%"
SET receiver_folder2="%receiver_root_folder%\%facility%\gpfs\%beamline2%\%year%\data\%beamtime_id2%"

set proxy_address="127.0.0.1:8400"

echo db.%beamtime_id1%_%data_source%.insert({dummy:1}) | %mongo_exe% %beamtime_id1%_%data_source%
echo db.%beamtime_id2%_%data_source%.insert({dummy:1}) | %mongo_exe% %beamtime_id2%_%data_source%

call start_services.bat

"%3" token -endpoint http://127.0.0.1:8400/asapo-authorizer -secret admin_token.key -type read %beamtime_id1% > token
set /P token1=< token
"%3" token -endpoint http://127.0.0.1:8400/asapo-authorizer -secret admin_token.key -type read %beamtime_id2% > token
set /P token2=< token



REM producer
mkdir %receiver_folder1%
mkdir %receiver_folder2%
start /B "" "%1" %proxy_address% %beamtime_id1% 100 1000 4 0 100
start /B "" "%1" %proxy_address% %beamtime_id2% 100 900 4 0 100
ping 192.0.2.1 -n 1 -w 1000 > nul

REM consumer
"%2" %proxy_address% %receiver_folder1% %beamtime_id1% 2 %token1% 12000  0 > out1.txt
type out1.txt
findstr /i /l /c:"Processed 1000 file(s)"  out1.txt || goto :error

"%2" %proxy_address% %receiver_folder2% %beamtime_id2% 2 %token2% 12000  0 > out2.txt
type out2.txt
findstr /i /l /c:"Processed 900 file(s)"  out2.txt || goto :error



goto :clean

:error
call :clean
exit /b 1

:clean
call stop_services.bat
rmdir /S /Q %receiver_root_folder%
del /f token1
del /f token2
echo db.dropDatabase() | %mongo_exe% %beamtime_id1%_%data_source%
echo db.dropDatabase() | %mongo_exe% %beamtime_id2%_%data_source%


