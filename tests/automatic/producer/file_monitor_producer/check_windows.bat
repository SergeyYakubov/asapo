SET root_folder=c:\tmp\asapo
set short_name="%~nx1"


ping 1.0.0.0 -n 1 -w 100 > nul

mkdir %root_folder%\test_in\test1
mkdir %root_folder%\test_in\test2
mkdir %root_folder%\test_out

start /B "" "%1" test.json > output
ping 1.0.0.0 -n 2 -w 100 > nul

echo test1 > %root_folder%\test_in\test1\test1.dat
echo test2 > %root_folder%\test_in\test2\test1.tmp

mkdir  %root_folder%\test_in\test2\subdir
echo test3 > %root_folder%\test_in\test2\subdir\test3.dat

ping 1.0.0.0 -n 2 -w 100 > nul

type %root_folder%\test_out\test1\test1.dat | findstr /c:"test1"  || goto :error
type %root_folder%\test_out\test2\subdir\test3.dat | findstr /c:"test3"  || goto :error

if  exist "%root_folder%\test_out\test2\test2.tmp" (
goto :error
)

if  exist "%root_folder%\test_in\test1\test1.dat" (
goto :error
)

if  exist "%root_folder%\test_in\test2\subdir\test3.dat" (
goto :error
)

rem type output
rem  type output | findstr /c:"Processed 2"  || goto :error

goto :clean

:error
call :clean
exit /b 1

:clean
Taskkill /IM "%short_name%" /F
rmdir /S /Q  %root_folder%\test_in
rmdir /S /Q  %root_folder%\test_out


