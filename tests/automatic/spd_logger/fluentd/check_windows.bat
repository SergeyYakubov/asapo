set folder=c:\opt\td-agent\logs
del %folder%\asapo.*.log

"%1"

ping 1.0.0.0 -n 5 > nul

findstr /I /L /C:"test info" %folder%\asapo.*.log || goto :error
findstr /I /L /C:"test error" %folder%\asapo.*.log || goto :error
findstr /I /L /C:"test debug" %folder%\asapo.*.log || goto :error
findstr /I /L /C:"test warning" %folder%\asapo.*.log || goto :error


goto :clean

:error
call :clean
exit /b 1

:clean
del output
