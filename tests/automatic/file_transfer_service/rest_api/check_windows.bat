SET file_transfer_folder=c:\\tmp\\asapo\\file_transfer\\files

c:\opt\consul\nomad run authorizer.nmd
c:\opt\consul\nomad run file_transfer.nmd

ping 1.0.0.0 -n 1 -w 100 > nul

set token=bnCXpOdBV90wU1zybEw1duQNSORuwaKz6oDHqmL35p0=

C:\Curl\curl.exe --silent --data "{\"Folder\":\"%file_transfer_folder%\",\"BeamtimeId\":\"aaa\",\"Token\":\"%token%\"}" 127.0.0.1:5007/folder > token
set /P folder_token=< token

mkdir %file_transfer_folder%

echo hello > %file_transfer_folder%\aaa

C:\Curl\curl.exe --silent -H "Authorization: Bearer %folder_token%" --data "{\"Folder\":\"%file_transfer_folder%\",\"FileName\":\"aaa\",\"Token\":\"%folder_token%\"}" 127.0.0.1:5008/transfer --stderr - | findstr hello  || goto :error

goto :clean

:error
call :clean
exit /b 1

:clean
c:\opt\consul\nomad stop authorizer
c:\opt\consul\nomad stop file_transfer
rmdir /S /Q %file_transfer_folder%
