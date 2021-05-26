SET mongo_exe="c:\Program Files\MongoDB\Server\4.2\bin\mongo.exe"
SET beamtime_id=asapo_test
SET beamline=test
SET stream1=s1
SET stream2=s2

SET receiver_root_folder=c:\tmp\asapo\receiver\files
SET receiver_folder="%receiver_root_folder%\test_facility\gpfs\%beamline%\2019\data\%beamtime_id%"


set proxy_address="127.0.0.1:8400"

echo db.%beamtime_id%_%stream1%.insert({dummy:1}) | %mongo_exe% %beamtime_id%_%stream1%
echo db.%beamtime_id%_%stream2%.insert({dummy:1}) | %mongo_exe% %beamtime_id%_%stream2%

"%3" token -endpoint http://127.0.0.1:8400/asapo-authorizer -secret admin_token.key -types read %beamtime_id% > token
set /P token=< token

REM producer
mkdir %receiver_folder%
start /B "" "%1" %proxy_address% %beamtime_id%%%%stream1% 100 1000 4 0 100
start /B "" "%1" %proxy_address% %beamtime_id%%%%stream2% 100 900 4 0 100
ping 192.0.2.1 -n 1 -w 1000 > nul

REM consumer
"%2" %proxy_address% %receiver_folder% %beamtime_id%%%%stream1% 2 %token% 12000  0 > out1.txt
type out1.txt
findstr /i /l /c:"Processed 1000 file(s)"  out1.txt || goto :error

"%2" %proxy_address% %receiver_folder% %beamtime_id%%%%stream2% 2 %token% 12000  0 > out2.txt
type out2.txt
findstr /i /l /c:"Processed 900 file(s)"  out2.txt || goto :error

goto :clean

:error
call :clean
exit /b 1

:clean
rmdir /S /Q %receiver_root_folder%
del /f token1
del /f token2
echo db.dropDatabase() | %mongo_exe% %beamtime_id%_%stream1%
echo db.dropDatabase() | %mongo_exe% %beamtime_id%_%stream2%


