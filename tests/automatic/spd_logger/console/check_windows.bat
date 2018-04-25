set full_recv_name="%2"
set short_recv_name="%~nx2"

start /B "" "%full_recv_name%" receiver.json

ping 1.0.0.0 -n 1 -w 100 > nul

mkdir files

%1 localhost:4200 100 1

ping 1.0.0.0 -n 1 -w 100 > nul

FOR /F "usebackq" %%A IN ('files\1.bin') DO set size=%%~zA

if %size% NEQ 102400 goto :error

goto :clean

:error
call :clean
exit /b 1

:clean
Taskkill /IM "%short_recv_name%" /F
rmdir /S /Q files
SET database_name=test_run
SET mongo_exe="c:\Program Files\MongoDB\Server\3.6\bin\mongo.exe"
echo db.dropDatabase() | %mongo_exe% %database_name%


