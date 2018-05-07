set full_recv_name="%2"
set short_recv_name="%~nx2"

start /B "" "%full_recv_name%"

ping 1.0.0.0 -n 1 -w 100 > nul

mkdir files

%1 localhost:4200 100 1

ping 1.0.0.0 -n 1 -w 100 > nul

FOR /F "usebackq" %%A IN ('files\0.bin') DO set size=%%~zA

if %size% NEQ 100 goto :error

goto :clean

:error
call :clean
exit /b 1

:clean
Taskkill /IM "%short_recv_name%" /F
rmdir /S /Q files

