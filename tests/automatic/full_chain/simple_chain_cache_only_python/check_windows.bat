SET mongo_exe="c:\Program Files\MongoDB\Server\4.2\bin\mongo.exe"
SET beamtime_id=asapo_test
SET data_source=python

set PYTHONPATH=%2:%3

"%1" "%4" %data_source% %beamtime_id%  "127.0.0.1:8400" %ASAPO_TEST_RW_TOKEN% > out || goto :error
type out

goto :clean

:error
call :clean
exit /b 1

:clean
echo db.dropDatabase() | %mongo_exe% %beamtime_id%_python


