SET mongo_exe="c:\Program Files\MongoDB\Server\4.2\bin\mongo.exe"
SET beamtime_id=asapo_test
SET beamline=test
SET data_source=python
SET receiver_root_folder=c:\tmp\asapo\receiver\files
SET receiver_folder="%receiver_root_folder%\test_facility\gpfs\%beamline%\2019\data\%beamtime_id%"
SET dbname=%beamtime_id%_%data_source%

mkdir %receiver_folder%

echo test > file1

ping 192.0.2.1 -n 1 -w 1000 > nul

set PYTHONPATH=%2

"%1" "%3" %data_source% %beamtime_id%  "127.0.0.1:8400" > out
type out
set NUM=0
for /F %%N in ('find /C "successfuly sent" ^< "out"') do set NUM=%%N
echo %NUM% | findstr 17 || goto error

for /F %%N in ('find /C "} error: wrong input, context: response:error: Bad request, message: already have record with same id" ^< "out"') do set NUM=%%N
echo %NUM% | findstr 2 || goto error

for /F %%N in ('find /C "} error: server warning, context: response:ignoring duplicate record" ^< "out"') do set NUM=%%N
echo %NUM% | findstr 2 || goto error

for /F %%N in ('find /C "} error: server warning, context: response:duplicated request" ^< "out"') do set NUM=%%N
echo %NUM% | findstr 1 || goto error


findstr /I /L /C:"Finished successfully" out || goto :error

goto :clean

:error
call :clean
exit /b 1

:clean
rmdir /S /Q %receiver_root_folder%
echo db.dropDatabase() | %mongo_exe% %dbname%


