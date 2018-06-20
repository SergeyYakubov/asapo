SET folder=files

mkdir %folder%

"%1" %folder% beamtime_id 11 4 4 1 10 2>&1 | findstr "Rate" || goto :error

FOR /F "usebackq" %%A IN ('%folder%\0.bin') DO set size=%%~zA
if %size% NEQ 11264 goto :error

FOR /F "usebackq" %%A IN ('%folder%\1.bin') DO set size=%%~zA
if %size% NEQ 11264 goto :error

FOR /F "usebackq" %%A IN ('%folder%\2.bin') DO set size=%%~zA
if %size% NEQ 11264 goto :error

FOR /F "usebackq" %%A IN ('%folder%\3.bin') DO set size=%%~zA
if %size% NEQ 11264 goto :error

goto :clean

:error
call :clean
exit /b 1

:clean
rmdir /S /Q %folder%

