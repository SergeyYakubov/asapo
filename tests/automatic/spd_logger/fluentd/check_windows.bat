set folder=c:\opt\td-agent\logs
del %folder%\asapo.*.log

"%1"

ping 1.0.0.0 -n 5 > nul

findstr /I /L /C:"test_info" %folder%\asapo.*.log || goto :error
findstr /I /L /C:"test_error" %folder%\asapo.*.log || goto :error
findstr /I /L /C:"test_debug" %folder%\asapo.*.log || goto :error
findstr /I /L /C:"test_warning" %folder%\asapo.*.log || goto :error


goto :clean

:error
call :clean
exit /b 1

:clean
del output
