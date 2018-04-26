"%1" > output

findstr /I /L /C:"[info] test_info" output || goto :error
findstr /I /L /C:"[error] test_error" output || goto :error
findstr /I /L /C:"[debug] test_debug"  output|| goto :error
findstr /I /L /C:"[warning] test_warning" output || goto :error
findstr /I /L /C:"test_info_mt_0" output || goto :error
findstr /I /L /C:"test_info_mt_1" output || goto :error
findstr /I /L /C:"test_info_mt_2" output || goto :error
findstr /I /L /C:"test_info_mt_3" output || goto :error
findstr /I /L /C:"test_logger" output || goto :error

REM error_lev
findstr /I /L /C:"test_error_errorlev" output || goto :error
findstr /I /L /C:"test_debug_errorlev" output  && goto :error
findstr /I /L /C:"test_info_errorlev" output && goto :error
findstr /I /L /C:"test_warning_errorlev" output && goto :error

REM warning_lev
findstr /I /L /C:"test_error_warninglev" output || goto :error
findstr /I /L /C:"test_debug_warninglev" output && goto :error
findstr /I /L /C:"test_info_warninglev" output && goto :error
findstr /I /L /C:"test_warning_warninglev" output  || goto :error

REM info_lev
findstr /I /L /C:"test_error_infolev" output || goto :error
findstr /I /L /C:"test_debug_infolev" output && goto :error
findstr /I /L /C:"test_info_infolev" output || goto :error
findstr /I /L /C:"test_warning_infolev" output || goto :error

REM none_lev
findstr /I /L /C:"nonelev" output && goto :error

goto :clean

:error
call :clean
exit /b 1

:clean
del output

