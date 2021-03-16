setlocal
SET beamtime_id=aaa
SET file_transfer_folder=%cd%\asap3\petra3\gpfs\p01\2019\data\%beamtime_id%
set file_transfer_folder=%file_transfer_folder:\=\\%


c:\opt\consul\nomad run authorizer.nmd
c:\opt\consul\nomad run file_transfer.nmd

ping 192.0.2.1 -n 1 -w 1000 > nul

set token=eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJqdGkiOiJjMTRhdTFiaXB0M2FzbzNoYzJvZyIsInN1YiI6ImJ0X2FhYSIsIkV4dHJhQ2xhaW1zIjp7IkFjY2Vzc1R5cGUiOiJyZWFkIn19.rvtEPZhvqwG91sod6-iBPCMUXWtMQtmFsqpXNv5HvFc

mkdir %file_transfer_folder%

C:\Curl\curl.exe --silent --data "{\"Folder\":\"%file_transfer_folder%\",\"BeamtimeId\":\"aaa\",\"Token\":\"%token%\"}" 127.0.0.1:5007/folder > token
set /P folder_token=< token

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
