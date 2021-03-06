"%1" > output

findstr /I /L /C:"\"level\":\"info\",\"message\":\"test info\"" output || goto :error
findstr /I /L /C:"\"level\":\"info\",\"message\":\"test : info\"" output || goto :error
findstr /I /L /C:"\"level\":\"error\",\"message\":\"test error\"" output || goto :error
findstr /I /L /C:"\"level\":\"debug\",\"message\":\"test debug\"" output || goto :error
findstr /I /L /C:"\"level\":\"warning\",\"message\":\"test warning\"" output || goto :error
findstr /I /L /C:"\"test_int\":2,\"test_double\":1.0" output || goto :error

findstr /I /L /C:"test info_mt_0" output || goto :error
findstr /I /L /C:"test info_mt_1" output || goto :error
findstr /I /L /C:"test info_mt_2" output || goto :error
findstr /I /L /C:"test info_mt_3" output || goto :error
findstr /I /L /C:"test_logger" output || goto :error

REM error_lev
findstr /I /L /C:"test error_errorlev" output || goto :error
findstr /I /L /C:"test debug_errorlev" output  && goto :error
findstr /I /L /C:"test info_errorlev" output && goto :error
findstr /I /L /C:"test warning_errorlev" output && goto :error

REM warning_lev
findstr /I /L /C:"test error_warninglev" output || goto :error
findstr /I /L /C:"test debug_warninglev" output && goto :error
findstr /I /L /C:"test info_warninglev" output && goto :error
findstr /I /L /C:"test warning_warninglev" output  || goto :error

REM info_lev
findstr /I /L /C:"test error_infolev" output || goto :error
findstr /I /L /C:"test debug_infolev" output && goto :error
findstr /I /L /C:"test info_infolev" output || goto :error
findstr /I /L /C:"test warning_infolev" output || goto :error

REM none_lev
findstr /I /L /C:"nonelev" output && goto :error

goto :clean

:error
call :clean
exit /b 1

:clean
del output

