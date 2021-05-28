setlocal
SET beamtime_id=aaa
SET file_transfer_folder=c:\tmp\asapo\asap3\petra3\gpfs\p01\2019\data\%beamtime_id%
set file_transfer_folder=%file_transfer_folder:\=\\%

set token=%BT_AAA_TOKEN%

mkdir %file_transfer_folder%

C:\Curl\curl.exe --silent --data "{\"Folder\":\"%file_transfer_folder%\",\"BeamtimeId\":\"aaa\",\"Token\":\"%token%\"}" 127.0.0.1:5007/v0.1/folder > token
set /P folder_token=< token

echo hello > %file_transfer_folder%\aaa
ping 192.0.2.1 -n 1 -w 1000 > nul

C:\Curl\curl.exe -v --silent -H "Authorization: Bearer %folder_token%" --data "{\"Folder\":\"%file_transfer_folder%\",\"FileName\":\"aaa\",\"Token\":\"%folder_token%\"}" 127.0.0.1:5008/v0.1/transfer
C:\Curl\curl.exe --silent -H "Authorization: Bearer %folder_token%" --data "{\"Folder\":\"%file_transfer_folder%\",\"FileName\":\"aaa\",\"Token\":\"%folder_token%\"}" 127.0.0.1:5008/v0.1/transfer --stderr - | findstr hello  || goto :error

goto :clean

:error
call :clean
exit /b 1

:clean
rmdir /S /Q %file_transfer_folder%
