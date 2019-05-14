SET folder=files

mkdir %folder%

"%1" %folder% beamtime_id 1

FOR /F "usebackq" %%A IN ('%folder%\beamtime_global.meta') DO set size=%%~zA
if %size% NEQ 5 goto :error

type %folder%\beamtime_global.meta | findstr /c:"hello"  || goto :error

goto :clean

:error
call :clean
exit /b 1

:clean
rmdir /S /Q %folder%

