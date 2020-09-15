SET folder=files

mkdir %folder%

"%1" %folder% beamtime_id 11 4 4 1 10 2>&1 | findstr "Rate" || goto :error

FOR /F "usebackq" %%A IN ('%folder%\processed\1') DO set size=%%~zA
if %size% NEQ 11000 goto :error

FOR /F "usebackq" %%A IN ('%folder%\processed\2') DO set size=%%~zA
if %size% NEQ 11000 goto :error

FOR /F "usebackq" %%A IN ('%folder%\processed\3') DO set size=%%~zA
if %size% NEQ 11000 goto :error

FOR /F "usebackq" %%A IN ('%folder%\processed\4') DO set size=%%~zA
if %size% NEQ 11000 goto :error

goto :clean

:error
call :clean
exit /b 1

:clean
rmdir /S /Q %folder%

