SET mongo_exe="c:\Program Files\MongoDB\Server\4.2\bin\mongo.exe"
SET beamtime_id=11111111
SET beamtime_id2=22222222
SET beamline=p07
SET stream=python
SET receiver_root_folder=c:\tmp\asapo\receiver\files
SET receiver_folder="%receiver_root_folder%\test_facility\gpfs\%beamline%\2019\data\%beamtime_id%"
SET receiver_folder2="%receiver_root_folder%\test_facility\gpfs\%beamline%\2019\data\%beamtime_id2%"
SET dbname=%beamtime_id%_%stream%
SET dbname2=%beamtime_id2%_%stream%
SET token=-pZmisCNjAbjT2gFBKs3OB2kNOU79SNsfHud0bV8gS4=

echo db.%dbname%.insert({dummy:1})" | %mongo_exe% %dbname%

call start_services.bat

mkdir %receiver_folder%
mkdir %receiver_folder2%

echo test > file1

ping 1.0.0.0 -n 1 -w 100 > nul

set PYTHONPATH=%2

"%1" "%3" %beamline% %token%  %stream% "127.0.0.1:8400" > out

type out
set NUM=0
for /F %%N in ('find /C "successfuly sent" ^< "out"') do set NUM=%%N
echo %NUM% | findstr 3 || goto error

for /F %%N in ('find /C "reauthorization" ^< "out"') do set NUM=%%N
echo %NUM% | findstr 1 || goto error

for /F %%N in ('find /C "} server warning: duplicated request" ^< "out"') do set NUM=%%N
echo %NUM% | findstr 1 || goto error

goto :clean

:error
call :clean
exit /b 1

:clean
call stop_services.bat
rmdir /S /Q %receiver_root_folder%
rmdir /S /Q %receiver_root_folder2%
echo db.dropDatabase() | %mongo_exe% %dbname%
echo db.dropDatabase() | %mongo_exe% %dbname2%


