SET mongo_exe="c:\Program Files\MongoDB\Server\4.2\bin\mongo.exe"
SET beamtime_id=asapo_test
SET beamline=test
SET stream=python
SET receiver_root_folder=c:\tmp\asapo\receiver\files
SET receiver_folder="%receiver_root_folder%\test_facility\gpfs\%beamline%\2019\data\%beamtime_id%"
SET dbname = %beamtime_id%_%stream%

echo db.%dbname%.insert({dummy:1})" | %mongo_exe% %dbname%

call start_services.bat

mkdir %receiver_folder%

echo test > file1

ping 192.0.2.1 -n 1 -w 1000 > nul

set PYTHONPATH=%2

"%1" "%3" %stream% %beamtime_id%  "127.0.0.1:8400" > out
type out
type out | findstr /c:"hello self callback" || goto error

goto :clean

:error
call :clean
exit /b 1

:clean
call stop_services.bat
rmdir /S /Q %receiver_root_folder%
echo db.dropDatabase() | %mongo_exe% %dbname%


