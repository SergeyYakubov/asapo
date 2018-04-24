REM receiver
set full_recv_name="%2"
set short_recv_name="%~nx2"
start /B "" "%full_recv_name%" receiver.json
ping 1.0.0.0 -n 1 -w 100 > nul

REM broker
set full_broker_name="%3"
set short_broker_name="%~nx3"
start /B "" "%full_broker_name%" broker.json
ping 1.0.0.0 -n 1 -w 100 > nul

REM producer
mkdir files
start /B "" "%1" localhost:4200 100 100
ping 1.0.0.0 -n 1 -w 100 > nul

REM worker
set broker_address="127.0.0.1:5005"
set broker_database_name="test_run"
"%4" %broker_address% %broker_database_name% 2 | findstr "Processed 100 file(s)"  || goto :error


goto :clean

:error
call :clean
exit /b 1

:clean
Taskkill /IM "%short_recv_name%" /F
Taskkill /IM "%short_broker_name%" /F
rmdir /S /Q files
SET mongo_exe="c:\Program Files\MongoDB\Server\3.6\bin\mongo.exe"
echo db.dropDatabase() | %mongo_exe% %broker_database_name%


