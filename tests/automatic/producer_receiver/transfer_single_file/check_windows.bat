SET mongo_exe="c:\Program Files\MongoDB\Server\3.6\bin\mongo.exe"
SET database_name=test_run
SET receiver_folder="c:\tmp\asapo\receiver\files"

echo db.%database_name%.insert({dummy:1})" | %mongo_exe% %database_name%


c:\opt\consul\nomad run receiver.nmd
c:\opt\consul\nomad run discovery.nmd

ping 1.0.0.0 -n 1 -w 100 > nul

mkdir %receiver_folder%

%1 localhost:5006 100 1 1 0

ping 1.0.0.0 -n 1 -w 100 > nul

FOR /F "usebackq" %%A IN ('%receiver_folder%\1.bin') DO set size=%%~zA
if %size% NEQ 102400 goto :error

goto :clean

:error
call :clean
exit /b 1

:clean
c:\opt\consul\nomad stop receiver
c:\opt\consul\nomad stop discovery
rmdir /S /Q %receiver_folder%
echo db.dropDatabase() | %mongo_exe% %database_name%


