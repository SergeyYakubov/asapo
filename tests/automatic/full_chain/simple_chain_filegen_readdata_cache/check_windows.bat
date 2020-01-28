

SET mongo_exe="c:\Program Files\MongoDB\Server\4.2\bin\mongo.exe"
SET beamtime_id=asapo_test
SET beamline=test
SET receiver_root_folder=c:\tmp\asapo\receiver\files
SET receiver_folder="%receiver_root_folder%\%beamline%\%beamtime_id%"

set producer_short_name="%~nx1"


"%3" token -secret auth_secret.key %beamtime_id% > token
set /P token=< token

set proxy_address="127.0.0.1:8400"

echo db.%beamtime_id%_detector.insert({dummy:1}) | %mongo_exe% %beamtime_id%_detector

c:\opt\consul\nomad run receiver.nmd
c:\opt\consul\nomad run authorizer.nmd
c:\opt\consul\nomad run discovery.nmd
c:\opt\consul\nomad run broker.nmd
c:\opt\consul\nomad run nginx.nmd

ping 1.0.0.0 -n 10 -w 100 > nul

REM producer
mkdir %receiver_folder%
mkdir  c:\tmp\asapo\test_in\test1
mkdir  c:\tmp\asapo\test_in\test2
start /B "" "%1" test.json

ping 1.0.0.0 -n 3 -w 100 > nul

echo hello1 > c:\tmp\asapo\test_in\test1\file1
echo hello2 > c:\tmp\asapo\test_in\test1\file2
echo hello3 > c:\tmp\asapo\test_in\test2\file2

ping 1.0.0.0 -n 10 -w 100 > nul


REM consumer
"%2" %proxy_address%  %receiver_folder% %beamtime_id% 2 %token% 1000 0 > out.txt
type out.txt
findstr /i /l /c:"Processed 3 file(s)" out.txt || goto :error
findstr /i /l /c:"hello1" out.txt || goto :error
findstr /i /l /c:"hello2" out.txt || goto :error
findstr /i /l /c:"hello3" out.txt || goto :error


goto :clean

:error
call :clean
exit /b 1

:clean
c:\opt\consul\nomad stop receiver
c:\opt\consul\nomad stop discovery
c:\opt\consul\nomad stop broker
c:\opt\consul\nomad stop authorizer
c:\opt\consul\nomad stop nginx
c:\opt\consul\nomad run nginx_kill.nmd  && c:\opt\consul\nomad stop -yes -purge nginx_kill
rmdir /S /Q %receiver_root_folder%
rmdir /S /Q c:\tmp\asapo\test_in\test1
rmdir /S /Q c:\tmp\asapo\test_in\test2
Taskkill /IM "%producer_short_name%" /F
del /f out.txt

del /f token
echo db.dropDatabase() | %mongo_exe% %beamtime_id%_detector


