SET mongo_exe="c:\Program Files\MongoDB\Server\4.2\bin\mongo.exe"
SET beamtime_id=asapo_test
SET beamline=test
SET stream=python
SET receiver_root_folder=c:\tmp\asapo\receiver\files
SET receiver_folder="%receiver_root_folder%\test_facility\gpfs\%beamline%\2019\data\%beamtime_id%"
SET dbname = %beamtime_id%_%stream%

echo db.%dbname%.insert({dummy:1})" | %mongo_exe% %dbname%

c:\opt\consul\nomad run receiver.nmd
c:\opt\consul\nomad run authorizer.nmd
c:\opt\consul\nomad run discovery.nmd
c:\opt\consul\nomad run nginx.nmd

ping 1.0.0.0 -n 5 -w 100 > nul

mkdir %receiver_folder%

echo test > file1

ping 1.0.0.0 -n 1 -w 100 > nul

set PYTHONPATH=%2

"%1" "%3" %stream% %beamtime_id%  "127.0.0.1:8400" > out
type out
set NUM=0
for /F %%N in ('find /C "successfuly sent" ^< "out"') do set NUM=%%N
echo %NUM% | findstr 8 || goto error

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
echo db.dropDatabase() | %mongo_exe% %dbname%


