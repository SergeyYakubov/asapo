del test_file

icacls test_noaccess /grant:r users:W
rmdir /S /Q test_noaccess
rmdir /S /Q folder