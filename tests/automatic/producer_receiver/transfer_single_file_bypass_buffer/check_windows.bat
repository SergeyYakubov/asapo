SET mongo_exe="c:\Program Files\MongoDB\Server\3.6\bin\mongo.exe"
SET beamtime_id=asapo_test
SET beamline=test
SET receiver_root_folder=c:\tmp\asapo\receiver\files
SET receiver_folder="%receiver_root_folder%\%beamline%\%beamtime_id%"


echo db.%beamtime_id%_detector.insert({dummy:1})" | %mongo_exe% %beamtime_id%_detector


c:\opt\consul\nomad run receiver.nmd
c:\opt\consul\nomad run authorizer.nmd
c:\opt\consul\nomad run discovery.nmd
c:\opt\consul\nomad run nginx.nmd

ping 1.0.0.0 -n 1 -w 100 > nul

mkdir %receiver_folder%

"%1" localhost:8400 %beamtime_id% 60000 1 1 0 30

ping 1.0.0.0 -n 1 -w 100 > nul

FOR /F "usebackq" %%A IN ('%receiver_folder%\1') DO set size=%%~zA
if %size% NEQ 60000000 goto :error

echo db.data.find({"_id":1}) |  %mongo_exe% %beamtime_id%_detector  > out
type out
type out | findstr /c:"\"buf_id\" : 0" || goto :error
type out | findstr /c:user_meta || goto :error

goto :clean

:error
call :clean
exit /b 1

:clean
c:\opt\consul\nomad stop receiver
c:\opt\consul\nomad stop discovery
c:\opt\consul\nomad stop nginx
c:\opt\consul\nomad run nginx_kill.nmd  && c:\opt\consul\nomad stop -yes -purge nginx_kill
c:\opt\consul\nomad stop authorizer
rmdir /S /Q %receiver_root_folder%
echo db.dropDatabase() | %mongo_exe% %beamtime_id%_detector


