SET file_transfer_folder=c:\\tmp\\asapo\\file_transfer\\files

c:\opt\consul\nomad run authorizer.nmd
c:\opt\consul\nomad run file_transfer.nmd

ping 1.0.0.0 -n 1 -w 100 > nul

mkdir %file_transfer_folder%
echo | set /p dummyName="hello" > %file_transfer_folder%\aaa

"%1"  127.0.0.1:5007 127.0.0.1:5008 %file_transfer_folder% aaa  || goto :error

type bbb | findstr /c:"hello"  || goto :error

goto :clean

:error
call :clean
exit /b 1

:clean
c:\opt\consul\nomad stop authorizer
c:\opt\consul\nomad stop file_transfer
rmdir /S /Q %file_transfer_folder%
del /f bbb

