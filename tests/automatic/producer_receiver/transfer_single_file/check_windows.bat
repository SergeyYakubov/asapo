SET mongo_exe="c:\Program Files\MongoDB\Server\3.6\bin\mongo.exe"
SET beamtime_id=asapo_test
SET beamline=test
SET receiver_root_folder=c:\tmp\asapo\receiver\files
SET receiver_folder="%receiver_root_folder%\%beamline%\%beamtime_id%"


echo db.%beamtime_id%.insert({dummy:1})" | %mongo_exe% %beamtime_id%


c:\opt\consul\nomad run receiver.nmd
c:\opt\consul\nomad run authorizer.nmd
c:\opt\consul\nomad run discovery.nmd
c:\opt\consul\nomad run nginx.nmd

ping 1.0.0.0 -n 1 -w 100 > nul

mkdir %receiver_folder%

"%1" localhost:8400 %beamtime_id% 100 1 1 0 30

ping 1.0.0.0 -n 1 -w 100 > nul

FOR /F "usebackq" %%A IN ('%receiver_folder%\1.bin') DO set size=%%~zA
if %size% NEQ 102400 goto :error

"%1" localhost:8400 wrong_id 100 1 1 0 2 2>1 | findstr /c:"authorization failed"  || goto :error

goto :clean

:error
call :clean
exit /b 1

:clean
c:\opt\consul\nomad stop receiver
c:\opt\consul\nomad stop discovery
c:\opt\consul\nomad stop nginx
c:\opt\consul\nomad stop authorizer
rmdir /S /Q %receiver_root_folder%
echo db.dropDatabase() | %mongo_exe% %beamtime_id%


