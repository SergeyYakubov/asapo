SET mongo_exe="c:\Program Files\MongoDB\Server\4.2\bin\mongo.exe"
SET beamtime_id=asapo_test
SET data_source=python
SET beamline=test
SET receiver_root_folder=c:\tmp\asapo\receiver\files
SET receiver_folder="%receiver_root_folder%\test_facility\gpfs\%beamline%\2019\data\%beamtime_id%"
SET receiver_folder_online="%receiver_root_folder%\beamline\%beamline%\current"

mkdir %receiver_folder%
mkdir %receiver_folder_online%

set PYTHONPATH=%2

"%1" "%3" %data_source% %beamtime_id%  "127.0.0.1:8400" > out
type out

FOR /F "usebackq" %%A IN ('%receiver_folder%\raw\python\file1') DO set size=%%~zA
if %size% NEQ 5 goto :error

FOR /F "usebackq" %%A IN ('%receiver_folder_online%\raw\python\file1') DO set size=%%~zA
if %size% NEQ 5 goto :error

goto :clean

:error
call :clean
exit /b 1

:clean
rmdir /S /Q %receiver_root_folder% %receiver_folder_online%
echo db.dropDatabase() | %mongo_exe% %beamtime_id%_python


