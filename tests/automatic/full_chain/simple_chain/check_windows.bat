SET mongo_exe="c:\Program Files\MongoDB\Server\3.6\bin\mongo.exe"
set broker_database_name=test_run
SET receiver_folder="c:\tmp\asapo\receiver\files"
set proxy_address="127.0.0.1:8400"

echo db.%broker_database_name%.insert({dummy:1}) | %mongo_exe% %broker_database_name%

c:\opt\consul\nomad run receiver.nmd
c:\opt\consul\nomad run discovery.nmd
c:\opt\consul\nomad run broker.nmd
c:\opt\consul\nomad run nginx.nmd

ping 1.0.0.0 -n 10 -w 100 > nul

REM producer
mkdir %receiver_folder%
start /B "" "%1" %proxy_address% 100 1000 4 0
ping 1.0.0.0 -n 1 -w 100 > nul

REM worker
"%2" %proxy_address% %broker_database_name% 2 | findstr /c:"Processed 1000 file(s)"  || goto :error


goto :clean

:error
call :clean
exit /b 1

:clean
c:\opt\consul\nomad stop receiver
c:\opt\consul\nomad stop discovery
c:\opt\consul\nomad stop broker
c:\opt\consul\nomad stop nginx
rmdir /S /Q %receiver_folder%
echo db.dropDatabase() | %mongo_exe% %broker_database_name%


